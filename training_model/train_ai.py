import os
from concurrent.futures import ThreadPoolExecutor, as_completed
import numpy as np
import torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import Dataset, DataLoader
import open3d as o3d

# ==========================================
# ARCHITECTURE POINTNET
# ==========================================
class PointNetSegmentation(nn.Module):
    def __init__(self):
        super(PointNetSegmentation, self).__init__()
        self.mlp1 = nn.Sequential(
            nn.Linear(3, 64), nn.ReLU(),
            nn.Linear(64, 128), nn.ReLU(),
            nn.Linear(128, 512), nn.ReLU()
        )
        self.segmentation_head = nn.Sequential(
            nn.Linear(512, 256), nn.ReLU(),
            nn.Dropout(0.3),
            nn.Linear(256, 2)  # [0: Décor, 1: Humain]
        )

    def forward(self, x):
        feat = self.mlp1(x)
        global_feat = torch.max(feat, dim=1, keepdim=True)[0]
        x = feat + global_feat
        x = self.segmentation_head(x)
        return x


def preparer_nuage(chemin, max_points=1024):
    pcd = o3d.io.read_point_cloud(chemin)

    if not pcd.is_empty():
        pcd, _ = pcd.remove_statistical_outlier(nb_neighbors=20, std_ratio=2.0)
        points_nettoyes = np.asarray(pcd.points)
    else:
        points_nettoyes = np.array([])

    if len(points_nettoyes) == 0:
        return None, None

    if len(points_nettoyes) > max_points:
        indices = np.random.choice(len(points_nettoyes), max_points, replace=False)
    else:
        indices = np.random.choice(len(points_nettoyes), max_points, replace=True)

    points_reels = points_nettoyes[indices]

    points_norm = points_reels - np.mean(points_reels, axis=0)
    dist_max = np.max(np.sqrt(np.sum(points_norm**2, axis=1)))
    if dist_max > 0:
        points_norm = points_norm / dist_max

    return points_norm, points_reels


def charger_un_fichier(args):
    """Charge et prépare un seul fichier .ply (utilisé par le ThreadPoolExecutor)."""
    chemin, est_humain, max_points = args
    points_norm, _ = preparer_nuage(chemin, max_points)
    if points_norm is None:
        return None

    labels = np.zeros(max_points, dtype=np.int64)
    if est_humain:
        hauteurs = points_norm[:, 2]
        seuil = np.percentile(hauteurs, 20)
        labels[hauteurs > seuil] = 1

    return (
        torch.tensor(points_norm, dtype=torch.float32),
        torch.tensor(labels, dtype=torch.long),
    )


# ==========================================
# DATASET
# ==========================================
class DatasetScans(Dataset):
    def __init__(self, max_points=1024):
        self.max_points = max_points
        self.donnees = []

        fichiers = []

        dataset_dir = os.environ.get("DATASET_DIR", "").strip()

        if dataset_dir and os.path.isdir(dataset_dir):
            print(f"[train_ai] Chargement du dataset depuis : {dataset_dir}")
            for root, _dirs, files in os.walk(dataset_dir):
                est_humain = "humain" in os.path.basename(root).lower()
                for f in files:
                    if f.lower().endswith(".ply"):
                        fichiers.append((os.path.join(root, f), est_humain))
        else:
            print("[train_ai] DATASET_DIR non défini, utilisation du dossier courant.")
            for f in os.listdir("."):
                if f.lower().endswith(".ply"):
                    fichiers.append((os.path.join(".", f), True))

        print(f"[train_ai] {len(fichiers)} fichier(s) .ply trouvé(s).")

        if len(fichiers) == 0:
            return

        print("[train_ai] Pré-chargement des nuages en mémoire (parallèle)...")
        taches = [(chemin, est_humain, max_points) for chemin, est_humain in fichiers]

        with ThreadPoolExecutor(max_workers=4) as executor:
            futures = {executor.submit(charger_un_fichier, t): t for t in taches}
            termines = 0
            for future in as_completed(futures):
                resultat = future.result()
                if resultat is not None:
                    self.donnees.append(resultat)

                termines += 1
                if termines % 10 == 0 or termines == len(taches):
                    print(f"[train_ai]   {termines}/{len(taches)} fichiers chargés...")

        print(f"[train_ai] {len(self.donnees)} nuage(s) valide(s) en mémoire.")

    def __len__(self):
        return min(len(self.donnees), 400)

    def __getitem__(self, idx):
        return self.donnees[idx]


# ==========================================
# MAIN
# ==========================================
if __name__ == "__main__":
    print("--- ENTRAÎNEMENT DE L'IA SUR LES SCANS ---")

    dataset = DatasetScans()

    if len(dataset.donnees) == 0:
        print("Erreur : Aucun fichier .ply trouvé ou valide.")
        exit(1)

    dataloader = DataLoader(dataset, batch_size=16, shuffle=True, num_workers=2)

    model = PointNetSegmentation()
    criterion = nn.CrossEntropyLoss()
    optimizer = optim.Adam(model.parameters(), lr=0.001)

    model.train()
    print(f"Apprentissage démarré sur {len(dataset.donnees)} fichier(s)...")

    for epoch in range(1, 16):  # 15 époques
        perte_totale = 0.0
        for scenes, labels in dataloader:
            optimizer.zero_grad()
            sorties = model(scenes)
            loss = criterion(sorties.view(-1, 2), labels.view(-1))
            loss.backward()
            optimizer.step()
            perte_totale += loss.item()
        print(f"Époque {epoch}/15 - Perte (Loss) : {perte_totale/len(dataloader):.4f}")

    torch.save(model.state_dict(), "modele_laser.pth")
    print("\nL'IA s'est entraînée sur les fichiers !")