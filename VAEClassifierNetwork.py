import torch
from VAENetwork import VAENetwork 
import torch.nn.functional as F

class VAEClassifierNetwork(torch.nn.Module):
    def __init__(self, input_dim, hidden_dim, latent_dim,num_classes):
        super().__init__()
        # load pretrained model for the VAE
        path = './vaemodel.pt'
        self.VAENet = torch.load(path)
        self.VAENet.eval()
        self.VAENet.train(False) # do not train vae pre-trained
        
        # attach classifier to VAE
        self.fc1 = torch.nn.Linear(latent_dim, 10) 
        self.fc2 = torch.nn.Linear(10, num_classes) 
        self.smax = torch.nn.Softmax(dim=1)
    
    def forward(self, x):
        _,latent,_,_ = self.VAENet(x)
        h1 = F.relu(self.fc1(latent)) 
        out = self.smax(self.fc2(h1)) 
        return out