import sys
import Utils
from VAENetwork import VAENetwork 
import torch
import torchvision
import torch.optim as optim
import torch.nn.functional as F
import numpy as np

device = torch.device("cuda:0" if (torch.cuda.is_available()) else "cpu") 

def main():
    train_loader, test_loader = Utils.get_train_test_loaders(batch_size=10)
    
    vaemodel = VAENetwork(x_dim=20531, h_dim1= 60, z_dim=10).to(device) 
    epochs = 50
    optimizer = optim.Adam(vaemodel.parameters())
    train(epochs, vaemodel, train_loader, test_loader, optimizer)
    PATH = "./vaemodel.pt"
    # save model 
    torch.save(vaemodel, PATH)

def train(epochs, vaemodel, train_loader, test_loader, optimizer): 
    vaemodel.train() # set it in train mode
    loss_l1 = torch.nn.L1Loss()
    for i in range(epochs):
        train_loss = 0
        for batch_idx, (data, _) in enumerate(train_loader):
            data = data.to(device) 
            optimizer.zero_grad() # clear gradients

            recon_batch, _, mu, log_var = vaemodel(data)
            loss = loss_function(recon_batch, data, mu, log_var,loss_l1) 
            loss.backward()
            train_loss += loss.item()
            optimizer.step()

            if batch_idx % 1000 == 0:
                print('Train Epoch: {} [{}/{} ({:.0f}%)]\tLoss: {:.6f}'.format(
                    epochs, batch_idx * len(data), len(train_loader.dataset),
                    100. * batch_idx / len(train_loader), loss.item() / len(data)))
        print('====> Epoch: {} Average loss: {:.4f}'.format(i, train_loss / len(train_loader.dataset)))

def loss_function(recon_x, x, mu, log_var, loss_l1):
    L1 = loss_l1(recon_x, x)
    KLD = -0.5 * torch.sum(1 + log_var - mu.pow(2) - log_var.exp()) 
    return L1 + KLD
    
if __name__ == "__main__": 
    sys.exit(int(main() or 0))
