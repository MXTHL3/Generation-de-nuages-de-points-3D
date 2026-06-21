import os
import numpy as np
import torch
import torch.nn as nn
import torch.nn.functional as F
from torch.utils.data import Dataset, DataLoader
import open3d as o3d
from model import PointNeXtClassifier


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

        if os.path.exists(root_dir):
            h_dir = os.path.join(root_dir, 'humain')
            if os.path.exists(h_dir):
                humains_files = [os.path.join(h_dir, f) for f in os.listdir(h_dir) if f.endswith('.ply')]
            
            nh_dir = os.path.join(root_dir, 'non-humain')
            if os.path.exists(nh_dir):
                non_humains_files = [os.path.join(nh_dir, f) for f in os.listdir(nh_dir) if f.endswith('.ply')]

        print(f"Dataset : {len(humains_files)} humains et {len(non_humains_files)} non-humains trouvés.")

        # Equilibrer car les fichiers non-humains ne sont pas nombreux
        if len(non_humains_files) > 0 and len(humains_files) > len(non_humains_files):
            multiplicateur = len(humains_files) // len(non_humains_files)
            non_humains_files = non_humains_files * max(1, multiplicateur)
            print(f" Equilibrer : {len(non_humains_files)} objets non-humains rajoutés.")

        for f in humains_files:
            self.files.append(f)
            self.labels.append(1)  # 1 : humain
        for f in non_humains_files:
            self.files.append(f)
            self.labels.append(0)  # 0 : non-humain

    def __len__(self):  
        # Combien on a de fichiers
        return len(self.files)

    def __getitem__(self, idx):
        file_path = self.files[idx]
        label = self.labels[idx]
        
        pcd = o3d.io.read_point_cloud(file_path)
        points = np.asarray(pcd.points)  # Matrice (nb points, 3)
        
        if len(points) == 0:  # Au cas où le fichier est vide
            points = np.random.rand(self.num_points, 3)

        # Matrice de taille identique
        if len(points) > self.num_points:
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
    EPOCHS = 20   # IA s'entraine 20 fois
    NUM_POINTS = 1024
    
    dataset = PointCloudDataset(root_dir='dataset', num_points=NUM_POINTS) # Charge et normalise les fichiers
    dataloader = DataLoader(dataset, batch_size=BATCH_SIZE, shuffle=True, drop_last=True)
    
    # S'entraine sur la carte graphique ou le processeur
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    print(f"Entraînement de PointNeXt sur : {device}\n")
    
    model = PointNeXtClassifier(num_classes=2).to(device)
    criterion = nn.CrossEntropyLoss()
    
    # Optimiseur AdamW + Weight Decay pour PointNeXt : empêche l'IA de surapprendre
    optimizer = torch.optim.AdamW(model.parameters(), lr=0.001, weight_decay=1e-4)
    
    model.train()
    for epoch in range(EPOCHS):
        running_loss = 0.0
        correct = 0
        total = 0
        
        for points, labels in dataloader:
            points, labels = points.to(device), labels.to(device)
            
            optimizer.zero_grad()  # Efface historique
            outputs = model(points)  # IA devine
            loss = criterion(outputs, labels)  # On note IA
            loss.backward()  # Calcule erreur
            optimizer.step()  # Ajuste neurones
            
            # Savoir si IA a juste ou faux
            running_loss += loss.item()
            _, predicted = torch.max(outputs.data, 1)
            total += labels.size(0)
            correct += (predicted == labels).sum().item()
            
        epoch_loss = running_loss / len(dataloader)
        epoch_acc = (correct / total) * 100
        print(f"Epoch [{epoch+1}/{EPOCHS}] - Loss: {epoch_loss:.4f} - Précision: {epoch_acc:.2f}%")
        
    # Enregistrer dans un fichier la configuration des neurones
    torch.save(model.state_dict(), "pointnext_human.pth")
    print("\nModèle PointNeXt sauvegardé sous 'pointnext_human.pth' ")

if __name__ == "__main__":
    train_model()
