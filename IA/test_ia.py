import os
import numpy as np
import torch
import torch.nn as nn
import torch.nn.functional as F
import open3d as o3d
from model import PointNeXtClassifier
from train_ia import charger_points_3d

# ==========================================
# FONCTION DE PREDICTION 
# ==========================================
def predict_ply(file_path, model, device, num_points=1024):
    categories = {0: "Non-Humain", 1: "Humain"} # Définir les labels 
    
    if os.path.exists(file_path):
        points = charger_points_3d(file_path)
        
        # Prétraitement et normalisation 
        if len(points) > num_points:
            choice = np.random.choice(len(points), num_points, replace=False)
            points = points[choice, :]
        else:
            choice = np.random.choice(len(points), num_points, replace=True)
            points = points[choice, :]
            
        centroid = np.mean(points, axis=0)
        points = points - centroid
        m = np.max(np.sqrt(np.sum(points**2, axis=1)))
        if m > 0:
            points = points / m
    else:
        print(f" Fichier '{file_path}' introuvable.")
        points = np.random.rand(num_points, 3)

    # Passage au format Tensor PyTorch 
    points = points.astype(np.float32).T
    points_tensor = torch.tensor(points).unsqueeze(0).to(device)
    
    # Test
    with torch.no_grad():
        outputs = model(points_tensor)
        probabilities = F.softmax(outputs, dim=1)
        confidence, predicted_idx = torch.max(probabilities, 1)
        
    return predicted_idx.item(), confidence.item() * 100  # Indice de la classe prédite et le score de confiance 

# ==========================================
# MAIN
# ==========================================
if __name__ == "__main__":
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    print(f"Inférence exécutée sur : {device}")

    model = PointNeXtClassifier(num_classes=2).to(device)
    
    # Avoir ce qu'à appris l'IA avant
    model_path = "pointnext_human.pth"
    if os.path.exists(model_path):
        model.load_state_dict(torch.load(model_path, map_location=device))
        print(f"Poids du modèle '{model_path}' chargés avec succès.")
    else:
        print(f" Fichier de poids '{model_path}' introuvable.")

    model.eval() # Toujours être en mode évaluation 

    categories = {0: "Non-Humain", 1: "Humain"} 
    valid_extensions = ('.ply', '.las')
    
    # Tests boucle sur les vrais dossiers 
    for class_name in ['humain', 'non-humain']:
        dossier = os.path.join("dataset", class_name)
        vrai_label = 1 if class_name == 'humain' else 0
        
        if os.path.exists(dossier):
            fichiers = [f for f in os.listdir(dossier) if f.endswith(valid_extensions)]
            print(f"\n--- ÉVALUATION DU DOSSIER : {class_name.upper()} ({len(fichiers)} fichiers) ---")
            
            reussites = 0
            for f in fichiers:
                chemin = os.path.join(dossier, f)
                pred, conf = predict_ply(chemin, model, device) # Renvoie la prédiction et la certitude
                
                status = " CORRECT" if pred == vrai_label else " ERREUR"
                if pred == vrai_label:
                    reussites += 1
                    
                print(f"Fichier: {f:20} -> Prédit: {categories[pred]:10} ({conf:.1f}%) | {status}")
            
            taux = (reussites / len(fichiers)) * 100 if fichiers else 0
            print(f"-> Score global pour {class_name} : {taux:.2f}% de bonnes réponses.")
        else:
            print(f"\nDossier introuvable : {dossier}")
