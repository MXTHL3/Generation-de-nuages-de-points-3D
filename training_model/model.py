import torch
import torch.nn as nn
import torch.nn.functional as F

# ==========================================
# ARCHITECTURE POINTNEXT 
# ==========================================
class SetAbstraction(nn.Module):
    def __init__(self, in_channel, mlp):
        super(SetAbstraction, self).__init__() # Hérite des réseaux de neurones
        self.mlp_convs = nn.ModuleList() # Liste convulsion
        self.mlp_bns = nn.ModuleList() # Liste normalisation
        last_channel = in_channel
        for out_channel in mlp:
            self.mlp_convs.append(nn.Conv1d(last_channel, out_channel, 1))    # Convolution
            self.mlp_bns.append(nn.BatchNorm1d(out_channel))    # Normalisation
            last_channel = out_channel

    def forward(self, xyz, points):
        # xyz: (Batch, 3, 1024), points: (Batch, caractéristiques, 1024)
        x = xyz 
        for i, conv in enumerate(self.mlp_convs):
            x = F.relu(self.mlp_bns[i](conv(x))) # Comprend les formes
        return x

class PointNeXtStage(nn.Module):
    def __init__(self, channels):
        # Bloc InvResBlock introduit par PointNeXt 
        super(PointNeXtStage, self).__init__()
        self.conv1 = nn.Conv1d(channels, channels, 1)
        self.bn1 = nn.BatchNorm1d(channels)
        self.conv2 = nn.Conv1d(channels, channels, 1)
        self.bn2 = nn.BatchNorm1d(channels)

    def forward(self, x):
        # Pour pas oublier des petits détails
        residual = x  
        x = F.relu(self.bn1(self.conv1(x)))  
        x = self.bn2(self.conv2(x))  
        x += residual  # Apprendre la différence manquante
        return F.relu(x)

class PointNeXtClassifier(nn.Module):
    def __init__(self, num_classes=2):
        super(PointNeXtClassifier, self).__init__()
        # Extraction des caractéristiques de base 
        self.sa = SetAbstraction(in_channel=3, mlp=[64, 128])
        
        # IA apprend les détails d'un humain
        self.stage1 = PointNeXtStage(128)
        self.stage2 = PointNeXtStage(128)
        
        # Global Abstraction PointNet 
        self.conv_b = nn.Conv1d(128, 512, 1)
        self.bn_b = nn.BatchNorm1d(512)
        
        # Classification MLP
        self.fc1 = nn.Linear(512, 256)
        self.bn_fc1 = nn.BatchNorm1d(256)
        self.drop1 = nn.Dropout(0.4) # Force IA à regarder humain en entier
        self.fc2 = nn.Linear(256, num_classes) # humain ou non-humain

    def forward(self, xyz):
        x = self.sa(xyz, None) # Savoir si c'est plat ou courbé
        
        # Formes plus complexes
        x = self.stage1(x)
        x = self.stage2(x) 
        
        #Comprendre les jambes, les bras, la tete, globalement
        x = F.relu(self.bn_b(self.conv_b(x))) 
        
        # Max Pooling global pour obtenir le vecteur de forme unique
        x = torch.max(x, 2, keepdim=True)[0]
        x = x.view(-1, 512)
        
        # Score des classes
        x = F.relu(self.bn_fc1(self.drop1(self.fc1(x))))
        x = self.fc2(x)
        return x