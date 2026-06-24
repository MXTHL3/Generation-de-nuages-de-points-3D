import os
import sys
import numpy as np
import torch
import torch.nn as nn
import torch.nn.functional as F
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
# FONCTION DE PREDICTION 
# ==========================================
def predict_ply(file_path, model, device, num_points=1024):
    categories = {0: "Non-Humain", 1: "Humain"}

    if os.path.exists(file_path):
        points = load_point_cloud(file_path)

        if len(points) == 0:
            print(f"[AVERTISSEMENT] Aucun point chargé depuis '{file_path}', prédiction aléatoire.")
            points = np.random.rand(num_points, 3)
        elif len(points) > num_points:
            choice = np.random.choice(len(points), num_points, replace=False)
            points = points[choice, :]
        else:
            choice = np.random.choice(len(points), num_points, replace=True)
            points = points[choice, :]

        # Normalisation
        centroid = np.mean(points, axis=0)
        points = points - centroid
        m = np.max(np.sqrt(np.sum(points**2, axis=1)))
        if m > 0:
            points = points / m
    else:
        print(f"[ERREUR] Fichier '{file_path}' introuvable.")
        points = np.random.rand(num_points, 3)

    # Passage au format Tensor PyTorch
    points = points.astype(np.float32).T
    points_tensor = torch.tensor(points).unsqueeze(0).to(device)

    with torch.no_grad():
        outputs = model(points_tensor)
        probabilities = F.softmax(outputs, dim=1)
        confidence, predicted_idx = torch.max(probabilities, 1)

    return predicted_idx.item(), confidence.item() * 100


# ==========================================
# MAIN
# ==========================================
if __name__ == "__main__":
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    print(f"Inférence exécutée sur : {device}")

    model = PointNeXtClassifier(num_classes=2).to(device)

    model_path = "pointnext_human_classifier.pth"
    if os.path.exists(model_path):
        model.load_state_dict(torch.load(model_path, map_location=device))
        print(f"Poids du modèle '{model_path}' chargés avec succès.")
    else:
        print(f"[ERREUR] Fichier de poids '{model_path}' introuvable.")

    model.eval()

    categories = {0: "Non-Humain", 1: "Humain"}

    fichier_unique = os.environ.get("PLY_PATH", "") or (sys.argv[1] if len(sys.argv) > 1 else "")
    if fichier_unique:
        EXTS = ('.ply', '.las', '.laz')
        if not fichier_unique.lower().endswith(EXTS):
            print(f"[ERREUR] Format non supporte : '{fichier_unique}'. Utilisez .ply, .las ou .laz.")
            sys.exit(1)
        print(f"\n--- ANALYSE DU FICHIER : {fichier_unique} ---")
        pred, conf = predict_ply(fichier_unique, model, device)
        print(f"Resultat    : {categories[pred]}")
        print(f"Confiance   : {conf:.1f}%")
        sys.exit(0)
    # ─────────────────────────────────────────────────────────────────────

    root = os.environ.get("DATASET_DIR", "")
    EXTS = ('.ply', '.las', '.laz')

    for class_name in ['humain', 'non-humain']:
        dossier = os.path.join(root, class_name)
        vrai_label = 1 if class_name == 'humain' else 0

        if os.path.exists(dossier):
            fichiers = [f for f in os.listdir(dossier) if f.lower().endswith(EXTS)]
            print(f"\n--- ÉVALUATION DU DOSSIER : {class_name.upper()} ({len(fichiers)} fichiers) ---")

            reussites = 0
            for f in fichiers:
                chemin = os.path.join(dossier, f)
                pred, conf = predict_ply(chemin, model, device)

                status = "✓ CORRECT" if pred == vrai_label else "✗ ERREUR"
                if pred == vrai_label:
                    reussites += 1

                print(f"Fichier: {f:20} -> Prédit: {categories[pred]:10} ({conf:.1f}%) | {status}")

            taux = (reussites / len(fichiers)) * 100 if fichiers else 0
            print(f"-> Score global pour {class_name} : {taux:.2f}% de bonnes réponses.")
        else:
            print(f"\nDossier introuvable : {dossier}")