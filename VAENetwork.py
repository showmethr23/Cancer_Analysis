import torch
import torch.nn as nn
import torch.nn.functional as F

class VAENetwork(nn.Module):
    def __init__(self, x_dim, h_dim1, z_dim):
        super(VAENetwork, self).__init__()
        # ----encoder components
        self.fce1 = nn.Linear(x_dim, h_dim1) 
        self.fcMu = nn.Linear(h_dim1, z_dim) 
        self.fcSigma = nn.Linear(h_dim1, z_dim) # ----decoder components
        self.fcd1 = nn.Linear(z_dim, h_dim1) 
        self.fcdout = nn.Linear(h_dim1, x_dim)
    
    def encoder(self, x):
        h = F.relu(self.fce1(x))
        return self.fcMu(h), self.fcSigma(h) # mu, log_var
    
    def reparameter_sampling(self, mu, log_var): 
        std = torch.exp(0.5*log_var)
        eps = torch.randn_like(std)
        return eps.mul(std).add_(mu) # z sample
    
    def decoder(self, z):
        h = F.relu(self.fcd1(z)) 
        return F.relu(self.fcdout(h))
    
    def forward(self, x):
        mu, log_var = self.encoder(x)
        z = self.reparameter_sampling(mu, log_var) #print(z)
        out = self.decoder(z)
        return out, z, mu, log_var # z is latent rep.
