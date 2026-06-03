# Lancez Blender  par le terminal si vous voulez la sortie stdout
# Selectionnez l'armature pas le mesh !
import bpy
import os

# Fichiers générés .ply sont écrits à la racine du .blend
out_dir = os.path.dirname(bpy.data.filepath)
# Le nom de base des fichiers .ply générés de préférence le nom de l'objet/le genre et le nom de l'animation (ex: "man_walk")
base_filename = "run"

# Récupérer l'objet contenant le mesh "linké" à l'armature
def get_obj_linked_with_armature(armature_obj):
    for obj in bpy.data.objects:
        if obj.type == 'MESH':
            # Le mesh a un modifier Armature de "armature_obj)
            for mod in obj.modifiers:
                if mod.type == 'ARMATURE' and mod.object == armature_obj:
                    return obj
    return None

# Récupération de l'armature sélectionné
armature = bpy.context.active_object

if not armature or not armature.animation_data.action:
    print("Erreur : l'armature n'a pas d'animations !")
else:
    
    action = armature.animation_data.action
    keyframes = set()
    
    # Récupérations des keyframes pour 1 animation
    if armature.animation_data.action:
        for layer in action.layers:
            for strip in layer.strips:
                if hasattr(strip, "channelbags"):
                    for channel in strip.channelbags:
                        for fcurve in channel.fcurves:
                            if hasattr(fcurve, "keyframe_points"):
                                for kp in fcurve.keyframe_points:
                                    # on récupère le timecode/numéro de la frame qui correspond à un keyframe
                                    keyframes.add(int(kp.co.x))
                else:
                    print("Pas de channelbags version Blender 5.1.1 requise ! Si le problème persiste se référer à la doc Blender")
                    
    sorted_keyframes = sorted(list(keyframes))
    
    print(f"Frames détectées : {sorted_keyframes}")
    
    scene = bpy.context.scene
    obj = get_obj_linked_with_armature(armature)
    
    if not obj:
        print("Erreur: pas trouvé de mesh lié à l'armature")
    
    i = 0
    
    for frame in sorted_keyframes:
        # on applique la transformation correspondant timecode/numéro de la frame trouvée
        scene.frame_set(frame)
        
        # On sélectionne uniquement l'objet à exporter
        bpy.ops.object.select_all(action='DESELECT')
        obj.select_set(True)
        bpy.context.view_layer.objects.active = obj
        
        ply_path = os.path.join(out_dir, f"{base_filename}_{i:02d}.ply")
        
        bpy.ops.wm.ply_export(
            filepath=ply_path,
            check_existing=True,
            export_selected_objects=True,
            export_normals=True, # A voir si utile
            export_triangulated_mesh=True,
            export_attributes=False,
            export_uv=False,
            ascii_format=True,
            )
        
        print(f"Frame {i} exportée vers {ply_path}")
        i += 1
    
    print(f"Génération des fichiers .ply à partir des frames terminées !")
            