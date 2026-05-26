import os
import numpy as np
import torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import Dataset, DataLoader
import open3d as o3d

# ==========================================
# PRÉPARATION, NETTOYAGE ET NORMALISATION
# ==========================================
def charger_nuage_points(chemin_ply, augmenter_data=False):
    # lit un fichier PLY avec Open3D, le nettoie, le normalise et l augmente
    if not os.path.exists(chemin_ply):
        return torch.rand(1024, 3, dtype=torch.float32)

    pcd = o3d.io.read_point_cloud(chemin_ply)
    
    if not pcd.is_empty():
        pcd.remove_statistical_outlier(nb_neighbors=20, std_ratio=2.0) #enlèves les bruits
        points = np.asarray(pcd.points)
    else:
        points = np.array([])
    
    if len(points) > 1024:
        indices = np.random.choice(len(points), 1024, replace=False)
        points = points[indices]
    elif len(points) < 1024 and len(points) > 0:
        indices = np.random.choice(len(points), 1024, replace=True)
        points = points[indices]
    else:
        return torch.rand(1024, 3, dtype=torch.float32)

    # NORMALISATION 
    # Centre 0,0,0
    points = points - np.mean(points, axis=0)
    # rayon de 1
    dist_max = np.max(np.sqrt(np.sum(points**2, axis=1)))
    if dist_max > 0:
        points = points / dist_max  # tout à la même échelle

    # AUGMENTATION DE DONNÉES 
    if augmenter_data:
        # rotation aléatoire autour de l axe Y pour que l ia voit le fichier sous différentes formes
        theta = np.random.uniform(0, 2 * np.pi)
        rotation_matrix = np.array([
            [np.cos(theta), 0, np.sin(theta)],
            [0, 1, 0],
            [-np.sin(theta), 0, np.cos(theta)]
        ])
        points = np.dot(points, rotation_matrix)
        
        # simuler vrai laser
        #bruit = np.random.normal(0, 0.02, size=points.shape)
        #points = points + bruit

    return torch.tensor(points, dtype=torch.float32)

# ==========================================
# ARCHITECTURE POINTNET
# ==========================================
class PointNetClassifieur(nn.Module):
    def __init__(self):  # préparer structure de données
        super(PointNetClassifieur, self).__init__() # pour avoir toutes les fonctions
        self.mlp1 = nn.Sequential(
            nn.Linear(3, 64), nn.ReLU(),
            nn.Linear(64, 128), nn.ReLU(),
            nn.Linear(128, 512), nn.ReLU()   # permet de savoir c est quelles formes
        )
        self.fc = nn.Sequential(  #décision finale
            nn.Linear(512, 256), nn.ReLU(),
            nn.Dropout(0.4), # 0.4 pour pas surapprentissage
            nn.Linear(256, 2)  #h ou nh
        )

    def forward(self, x):   #flux de données
        x = self.mlp1(x) 
        x = torch.max(x, dim=1)[0] # Global Max Pooling matrice décision finale
        x = self.fc(x)  # matrice décision finale
        return x

# ==========================================
# DATASET 
# ==========================================
class DatasetScannerFictif(Dataset):
    def __init__(self, num_echantillons=400): # plus d exemples
        self.num_echantillons = num_echantillons
        self.fichiers_humains = ["human.ply"]
        self.fichiers_non_humains = [ "gargole.ply"]
        
    def __len__(self):
        return self.num_echantillons
        
    def __getitem__(self, idx):
        classe = np.random.choice([0, 1])
        
        if classe == 1:
            fichier = np.random.choice(self.fichiers_humains)
        else:
            fichier = np.random.choice(self.fichiers_non_humains)
            
        # augmentation de données pendant l entrainement
        points = charger_nuage_points(fichier, augmenter_data=True)
        return points, torch.tensor(classe, dtype=torch.long)

# =========================================
# BOUCLE ENTRAINEMENT 
# =========================================
if __name__ == "__main__":
    print("--- Début de l'entraînement IA ---")
    
    dataset = DatasetScannerFictif(num_echantillons=400)
    dataloader = DataLoader(dataset, batch_size=32, shuffle=True)  #paquets 32 et on mélange les fichiers
    
    model = PointNetClassifieur()
    criterion = nn.CrossEntropyLoss() # à quel point mon ia c est trompé
    optimizer = optim.Adam(model.parameters(), lr=0.001)
    
    print("Entraînement en cours...")
    model.train()
    for epoch in range(1, 6): # 5 époques
        perte_totale = 0.0
        for nuages, labels in dataloader:
            optimizer.zero_grad()
            sorties = model(nuages)  # prédiction
            loss = criterion(sorties, labels)
            loss.backward()  # on regarde où ça s est trompé
            optimizer.step()
            perte_totale += loss.item()
            
        print(f"Époque {epoch}/5 - Perte (Loss): {perte_totale/len(dataloader):.4f}")
        
    print("\n--- Entraînement terminé ---")
    torch.save(model.state_dict(), "modele_laser.pth")
    print("Modèle sauvegardé sous le nom : 'modele_laser.pth'")
