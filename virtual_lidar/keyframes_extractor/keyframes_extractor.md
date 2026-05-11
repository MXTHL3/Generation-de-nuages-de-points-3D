<!-- Antoine PLUQUET le 11/05/26 -->
# Extractions des keyframes à partir d'un objet avec une animation :

## Etape 1:
Importez votre **fichier objet (.fbx)** dans Blender (File->Import->FBX).  
Enregistrez ensuite votre fichier Blender où vous le souhaitez.

## Etape 2:
Sélectionnez dans la hiérarchie de la scène l'**armature**.

## Etape 3:
Allez dans l'onglet **Scripting** de Blender puis ouvrez le script **animation_blender.py** (Text->Open).  
Executez ensuite le code.

Bravo ! Vous devriez avoir en sortie les fichiers .ply correspondant à toutes les **keyframes** trouvées dans l'animation de votre fichier .fbx.

