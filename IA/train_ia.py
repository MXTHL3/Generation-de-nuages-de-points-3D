import os
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
            nn.Linear(256, 2) # [0: Décor, 1: Humain]
        )

    def forward(self, x):
        feat = self.mlp1(x)
        global_feat = torch.max(feat, dim=1, keepdim=True)[0]
        x = feat + global_feat 
        x = self.segmentation_head(x)
        return x


def preparer_nuage(chemin, max_points=1024):
    pcd = o3d.io.read_point_cloud(chemin)
        
    # nettoyage du bruit pour pas que PointNet fasse une forme globale du nuage immense
    if not pcd.is_empty():
        pcd.remove_statistical_outlier(nb_neighbors=20, std_ratio=2.0)
        points_nettoyes = np.asarray(pcd.points)
    else:
        points_nettoyes = np.array([])

    if len(points_nettoyes) == 0:
        return None, None

    # échantillonnage à 1024
    if len(points_nettoyes) > max_points:
         indices = np.random.choice(len(points_nettoyes), max_points, replace=False)
    else:
        indices = np.random.choice(len(points_nettoyes), max_points, replace=True)
        
    points_reels = points_nettoyes[indices]

    # Normalisation géométrique
    points_norm = points_reels - np.mean(points_reels, axis=0) #centré
    dist_max = np.max(np.sqrt(np.sum(points_norm**2, axis=1)))
    if dist_max > 0:
        points_norm = points_norm / dist_max   # tout à la même échelle
        
    return points_norm, points_reels

# ==========================================
# DATASET
# ==========================================
class DatasetScans(Dataset):
    def __init__(self, dossier=".", max_points=1024):
        self.max_points = max_points
        # on liste tous les fichiers pour s entrainer
        self.fichiers = [os.path.join(dossier, f) for f in os.listdir(dossier) 
                         if f.lower().endswith(".ply")]
        
    def __len__(self):
        return min(len(self.fichiers), 400)
        
    def __getitem__(self, idx):
        chemin = self.fichiers[idx]
        points_norm, _ = preparer_nuage(chemin, self.max_points)
        
        if points_norm is None:
            return torch.rand(self.max_points, 3), torch.zeros(self.max_points, dtype=torch.long)

        labels = np.zeros(self.max_points, dtype=np.long)
        hauteurs = points_norm[:, 2] # axe z donc la hauteur pour humain

        # tous les points qui se trouvent au-dessus de ces 20 % sont humains
        seuil_hauteur = np.percentile(hauteurs, 20) 
        labels[hauteurs > seuil_hauteur] = 1

        return torch.tensor(points_norm, dtype=torch.float32), torch.tensor(labels, dtype=torch.long)

# ==========================================
# MAIN
# ==========================================
if __name__ == "__main__":
    print("--- ENTRAÎNEMENT DE L'IA SUR LES SCANS ---")
    dataset = DatasetScans(dossier=".")
    
    if len(dataset.fichiers) == 0:
        print("Erreur : Aucun fichier .ply trouvé.")
        exit()
        
    dataloader = DataLoader(dataset, batch_size=16, shuffle=True)
    
    model = PointNetSegmentation()
    criterion = nn.CrossEntropyLoss()
    optimizer = optim.Adam(model.parameters(), lr=0.001)
    
    model.train()
    print(f"Apprentissage démarré sur {len(dataset.fichiers)} fichiers...")
    for epoch in range(1, 16): #15 époques
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
