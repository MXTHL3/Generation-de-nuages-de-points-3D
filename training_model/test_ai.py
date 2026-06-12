import os
import torch
import torch.nn as nn
import open3d as o3d
import numpy as np
from sklearn.cluster import DBSCAN
from train_ai import PointNetSegmentation, preparer_nuage

def compter_silhouettes_dans_scene(chemin_fichier, modele):
    try:
        points_normalises, points_humains_reels_base = preparer_nuage(chemin_fichier, max_points=1024)

        if points_normalises is None:
            return "Fichier vide ou invalide"

        # prédictions IA
        tenseur_points = torch.tensor(points_normalises, dtype=torch.float32).unsqueeze(0)
        with torch.no_grad(): # enlève la mémore
            predictions = modele(tenseur_points) # 0 ou 1 dans matrice
            tags_points = torch.argmax(predictions, dim=2).squeeze(0).numpy() # prend le score le plus haut

        # Extraction échelle réelle 
        points_humains_reels = points_humains_reels_base[tags_points == 1]

        # si on a moins de 230 sur 1024 c est un objet non-humain direct
        if len(points_humains_reels) < 30:
            return "0 human"

        # DBSCAN devine combien on a de groupes
        # 60 cm entre groupe et groupe contient min 15 pts
        clusteriseur = DBSCAN(eps=0.60, min_samples=15).fit(points_humains_reels)
        labels_clusters = clusteriseur.labels_ # met des labels pour chaqu egroupe

        groupes_distincts = set(labels_clusters) - {-1}
        
        centres_groupes = []
        for g in groupes_distincts:
            pts_groupe = points_humains_reels[labels_clusters == g]
            
            # calcule la hauteur 
            hauteur_reelle = np.max(pts_groupe[:, 2]) - np.min(pts_groupe[:, 2])
            
            if hauteur_reelle >= 0.80 and len(pts_groupe) >= 30: 
                centres_groupes.append(np.mean(pts_groupe, axis=0))

        if len(centres_groupes) == 0:
            return "0 human"

        # fusion des custers une personne trop proche 
        silhouettes_finales = []
        for centre in centres_groupes:
            trouve_voisin = False
            for autre_centre in silhouettes_finales:
                if np.linalg.norm(centre - autre_centre) < 0.90: 
                    trouve_voisin = True
                    break
            if not trouve_voisin:
                silhouettes_finales.append(centre)

        return f"{len(silhouettes_finales)} human"

    except Exception as e:
        return f"Erreur d'analyse : {e}"

if __name__ == "__main__":
    nom_modele = "modele_laser.pth"
    if not os.path.exists(nom_modele):
        print(f"Erreur : '{nom_modele}' introuvable.")
        exit()

    modele = PointNetSegmentation()
    modele.load_state_dict(torch.load(nom_modele))
    modele.eval()

    ply_path = os.environ.get("PLY_PATH", "").strip()
    
    if ply_path and os.path.isfile(ply_path):
        verdict = compter_silhouettes_dans_scene(ply_path, modele)
        print(verdict)  
    else:
        print("--- RECONNAISSANCE GLOBALE SUR LE DATASET ---")
        dossier_scans = "."
        for nom_fichier in sorted(os.listdir(dossier_scans)):
            if nom_fichier.lower().endswith(".ply"):
                chemin_complet = os.path.join(dossier_scans, nom_fichier)
                verdict = compter_silhouettes_dans_scene(chemin_complet, modele)
                print(f"Fichier : {nom_fichier:<30} -> {verdict}")
