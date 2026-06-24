import os
import numpy as np
import torch
import torch.nn as nn
import torch.nn.functional as F
from torch.utils.data import Dataset, DataLoader
import open3d as o3d
import laspy
from model import PointNeXtClassifier


# ==========================================
# LECTURE DES NUAGES DE POINTS
# ==========================================
def load_point_cloud(file_path: str) -> np.ndarray:
    ext = os.path.splitext(file_path)[1].lower()

    if ext in ('.las', '.laz'):
        try:
            with laspy.open(file_path) as f:
                las = f.read()
            points = np.stack([
                np.asarray(las.x),
                np.asarray(las.y),
                np.asarray(las.z),
            ], axis=1).astype(np.float64)
            return points
        except ImportError:
            print("[ERREUR] Le module 'laspy' est introuvable. Installez-le avec : pip install laspy[lazrs]")
            return np.empty((0, 3))
        except Exception as e:
            print(f"[ERREUR] Impossible de lire '{file_path}' avec laspy : {e}")
            return np.empty((0, 3))

    # Formats natifs Open3D (PLY, PCD, XYZ…)
    pcd = o3d.io.read_point_cloud(file_path)
    points = np.asarray(pcd.points)
    if len(points) == 0:
        print(f"[AVERTISSEMENT] Open3D n'a lu aucun point dans '{file_path}'.")
    return points


# ==========================================
# DATASET
# ==========================================
class PointCloudDataset(Dataset):
    def __init__(self, root_dir, num_points=1024):
        self.root_dir = root_dir
        self.num_points = num_points
        self.files = []
        self.labels = []
        self.class_map = {'non-humain': 0, 'humain': 1}

        humains_files = []
        non_humains_files = []
        EXTS = ('.ply', '.las', '.laz')

        if os.path.exists(root_dir):
            h_dir = os.path.join(root_dir, 'humain')
            if os.path.exists(h_dir):
                humains_files = [os.path.join(h_dir, f) for f in os.listdir(h_dir) if f.lower().endswith(EXTS)]

            nh_dir = os.path.join(root_dir, 'non-humain')
            if os.path.exists(nh_dir):
                non_humains_files = [os.path.join(nh_dir, f) for f in os.listdir(nh_dir) if f.lower().endswith(EXTS)]

        print(f"Dataset : {len(humains_files)} humains et {len(non_humains_files)} non-humains trouvés.")

        # Equilibrer car les fichiers non-humains ne sont pas nombreux
        if len(non_humains_files) > 0 and len(humains_files) > len(non_humains_files):
            multiplicateur = len(humains_files) // len(non_humains_files)
            non_humains_files = non_humains_files * max(1, multiplicateur)
            print(f"Equilibrer : {len(non_humains_files)} objets non-humains après duplication.")

        for f in humains_files:
            self.files.append(f)
            self.labels.append(1)  # 1 : humain
        for f in non_humains_files:
            self.files.append(f)
            self.labels.append(0)  # 0 : non-humain

    def __len__(self):
        return len(self.files)

    def __getitem__(self, idx):
        file_path = self.files[idx]
        label = self.labels[idx]

        points = load_point_cloud(file_path)

        if len(points) == 0:  # Fichier illisible ou vide
            print(f"[AVERTISSEMENT] Aucun point dans '{file_path}', remplacement aléatoire.")
            points = np.random.rand(self.num_points, 3)
        elif len(points) > self.num_points:
            choice = np.random.choice(len(points), self.num_points, replace=False)
            points = points[choice, :]
        else:
            choice = np.random.choice(len(points), self.num_points, replace=True)
            points = points[choice, :]

        # Normalisation
        centroid = np.mean(points, axis=0)
        points = points - centroid
        m = np.max(np.sqrt(np.sum(points**2, axis=1)))
        if m > 0:
            points = points / m

        # Faire en sorte que les fichiers non-humain dupliqués soient différents
        if label == 0:
            points += np.random.normal(0, 0.01, points.shape)

        points = points.astype(np.float32).T
        return torch.tensor(points), torch.tensor(label, dtype=torch.long)


# ==========================================
# BOUCLE D'ENTRAÎNEMENT
# ==========================================
def train_model():
    BATCH_SIZE = 8
    EPOCHS = 20
    NUM_POINTS = 1024

    root = os.environ.get("DATASET_DIR", "dataset")
    print(f"DATASET_DIR reçu : '{root}'")
    print(f"Contenu : {os.listdir(root) if os.path.exists(root) else 'DOSSIER INTROUVABLE'}")

    dataset = PointCloudDataset(root_dir=root, num_points=NUM_POINTS)
    if len(dataset) == 0:
        print("Erreur : aucun fichier trouvé dans le dataset. Vérifiez les dossiers humain/ et non-humain/.")
        return

    actual_batch = min(BATCH_SIZE, len(dataset))
    dataloader = DataLoader(dataset, batch_size=actual_batch, shuffle=True, drop_last=False)

    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    print(f"Entraînement de PointNeXt sur : {device}\n")

    model = PointNeXtClassifier(num_classes=2).to(device)
    criterion = nn.CrossEntropyLoss()
    optimizer = torch.optim.AdamW(model.parameters(), lr=0.001, weight_decay=1e-4)

    model.train()
    for epoch in range(EPOCHS):
        running_loss = 0.0
        correct = 0
        total = 0

        for points, labels in dataloader:
            points, labels = points.to(device), labels.to(device)

            optimizer.zero_grad()
            outputs = model(points)
            loss = criterion(outputs, labels)
            loss.backward()
            optimizer.step()

            running_loss += loss.item()
            _, predicted = torch.max(outputs.data, 1)
            total += labels.size(0)
            correct += (predicted == labels).sum().item()

        epoch_loss = running_loss / len(dataloader)
        epoch_acc = (correct / total) * 100
        print(f"Epoch [{epoch+1}/{EPOCHS}] - Loss: {epoch_loss:.4f} - Précision: {epoch_acc:.2f}%")

    torch.save(model.state_dict(), "pointnext_human_classifier.pth")
    print("\nModèle PointNeXt sauvegardé sous 'pointnext_human_classifier.pth'")


if __name__ == "__main__":
    train_model()