# Projet TER 2026 : Génération de nuages de points 3D

### Résumé :

Ce projet est une application dédiée à l'analyse de nuage de points autour de deux parties :
 - la génération et la visualisation de nuages de points via le scan par lidar virtuel d'une scène modifiable.

 - l'entrainement d'un modèle de reconnaissance qui a pour but de reconnaitre des sihouettes humaines au sein de nuages de points.

### Membres :

- PLUQUET Antoine antoine.pluquet@etu.univ-amu.fr
- QUFAJ Vanessa vanessa.qufaj@etu.univ-amu.fr
- THIEL Maxime maxime.thiel@etu.univ-amu.fr

## Prérequis

Le projet a été développé et testé sous Linux (Ubuntu 22.04+).

### Dépendances système

Installer les paquets suivants :

```bash
sudo apt update

sudo apt install -y \
    build-essential \
    g++ \
    make \
    pkg-config \
    libgtkmm-3.0-dev \
    libepoxy-dev \
    libpdal-dev \
    pdal \
    libpng-dev \
    libcgal-dev \
    libgmp-dev \
    libmpfr-dev \
    libspdlog-dev \
    libfmt-dev \
    libglm-dev \
    libeigen3-dev \
    nlohmann-json3-dev \
    python3 \
    python3-pip \
    python3-dev

pip3 install torch torchvision --index-url https://download.pytorch.org/whl/cpu
pip3 install open3d numpy