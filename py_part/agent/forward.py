import numpy as np
import torch
from torch import nn
from torch.autograd import Variable


class Swish(nn.Module):
    """Relu激活函数变种"""
    def __init__(self):
        super().__init__()

    def forward(self, x):
        return torch.mul(x, torch.sigmoid(x))

class DuelingDQN(nn.Module):
    def __init__(self, image_shape, output_size):
        super().__init__()
        in_channels, in_height, in_width = image_shape
        print((image_shape))
        print((output_size))

        self.swish = Swish()

        self.filter1 = nn.Conv2d(in_channels, out_channels=64,kernel_size=(4, 4), stride=(2, 2))
        self.filter2 = nn.Conv2d(in_channels=64, out_channels=64,kernel_size=(2, 2), stride=(1, 1))
        #self.filter3 = nn.Conv2d(in_channels=128, out_channels=128,kernel_size=(2, 2), stride=(1, 1))

        filter_output_shape = self._get_filter_size(image_shape)
        self.linear = nn.Linear(filter_output_shape, 512)

        self.A = nn.Linear(512, output_size)
        self.V = nn.Linear(512, 1)

    def forward(self, x):
        net = self._build_common_network(x)
        value = self.V(net)
        advantage = self.A(net)
        return value + advantage - torch.mean(advantage)

    def _get_filter_size(self, shape):
        batch = 1
        x = Variable(torch.rand(batch, *shape))
        #net = self.filter3(self.filter2(self.filter1(x)))
        net = self.filter2(self.filter1(x))
        return int(np.prod(net.size()[1:]))

    def _build_common_network(self, x):
        net = self.swish(self.filter1(x))
        net = self.swish(self.filter2(net))
        #net = self.swish(self.filter3(net))
        net = net.view(net.size(0), -1)
        return self.swish(self.linear(net))











