import os
import torch
import torch.nn as nn
import open3d as o3d
import numpy as np
from train_ai import PointNetClassifieur, charger_nuage_points

def predire_un_fichier(chemin_fichier, modele):
    #Prend un chemin de fichier PLY et renvoie la chaîne de caractères du verdict
    try:
        # On appelle la fonction de train
        tenseur_points = charger_nuage_points(chemin_fichier, augmenter_data=False)
        
        # On ajoute la dimension de Batch requise par l'IA (1, 1024, 3)
        tenseur_points = tenseur_points.unsqueeze(0)

        # Prédiction
        with torch.no_grad():
            prediction = modele(tenseur_points)  # matrice
            classe_predite = torch.argmax(prediction, dim=1).item()  # cherche les score maw

        classes_noms = {0: "Non-Humain", 1: "Humain"}
        return classes_noms[classe_predite]

    except Exception as e:
        return f"Erreur d'analyse : {e}"

# ==========================================
# BOUCLE AUTOMATIQUE SUR LE DOSSIER
# ==========================================
if __name__ == "__main__":
    nom_modele = os.environ.get("MODELE_PATH", "modele_laser.pth")
 
    if not os.path.exists(nom_modele):
        print(f"Erreur : Le fichier de poids '{nom_modele}' est introuvable. "
              "Lancer d'abord train_ai.py pour générer 'modele_laser.pth'.")
        exit(1)
 
    # chargement du modèle
    modele = PointNetClassifieur()
    modele.load_state_dict(torch.load(nom_modele, map_location="cpu"))
    modele.eval()
 
    ply_path = os.environ.get("PLY_PATH", "")
 
    if ply_path:
        if not os.path.exists(ply_path):
            print(f"Erreur : Fichier introuvable : {ply_path}")
            exit(1)
 
        verdict = predire_un_fichier(ply_path, modele)
        print(verdict)  
 
    else:
        dossier_scans = "."
        print("--- VÉRIFICATION AUTOMATIQUE DES NUAGES DE POINTS ---")
        print(f"Analyse du dossier : {os.path.abspath(dossier_scans)}\n")
 
        compteur_ply = 0
        for nom_fichier in sorted(os.listdir(dossier_scans)):
            if nom_fichier.lower().endswith(".ply"):
                compteur_ply += 1
                chemin_complet = os.path.join(dossier_scans, nom_fichier)
                verdict = predire_un_fichier(chemin_complet, modele)
                print(f"[{compteur_ply}] Fichier : {nom_fichier:<25} -> Verdict : {verdict}")
 
        if compteur_ply == 0:
            print("Aucun fichier .ply trouvé dans ce dossier.")
