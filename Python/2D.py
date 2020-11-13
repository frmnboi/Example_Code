'''
Created on May 29, 2019

@author: Daniel Foreman
'''

import numpy as np
import matplotlib.pyplot as plt

#paintball problem
#alpha is the position of the shooter x
#beta is the position of the shooter y

#priors, right now, all are equally likely
alphas=np.linspace(0.0001,30,300)
#alphas=np.array([10])

betas=np.linspace(0.0001,20,200)
#betas=np.array([20])

#Likelyhood of the spots on the wall given the person is firing randomly

#theta=invtan((x-alpha)/beta)
#dxdtheta=b/cos^2(theta)

def Normalize(arr):
    return arr/np.sum(arr)

def StrafingSpeed(alpha,beta,x):
    #alpha and beta are the shooter coordinates
    #x is the splatter location
    theta=np.arctan2.outer(np.subtract.outer(x,alpha),beta)
    speed=beta/np.cos(theta)**2 #derivative of x with respect to theta
    return speed

def locationProbDist(alpha,beta,locations):
    dist=1/StrafingSpeed(alpha, beta, locations)
    return Normalize(dist)

#locations=[1,2,3,5,8,13,21,29]
#locations=np.random.rand(50)*len(alphas)
#locations=[25]
#locations=[1]*17+[29]*17
#locations=[10]*10+[18]*10
#locations=[15,3,17,23,7,11,12,4,25]
#locations=[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30]*2
locations=[0,15,30]*14
locations=[0,30]*14    
data=locationProbDist(alphas, betas, locations)

#the first element in data is the dimension of the actual data we observed.  We want to multiply through
grid=np.prod(data,axis=0)
#too many values will cause it to have discretization error
#will require a custom apply along axis function that normlizes as it multiplies if it's too small to keep values from dissapearing.


#uniform prior
prior=np.ones([alphas.shape[0],betas.shape[0]])


posterior=Normalize(np.multiply(prior,grid))

    
#plt.plot(data.reshape(locations.shape))
plt.imshow(posterior, cmap='hot', interpolation='nearest',extent=[min(betas),max(betas),min(alphas),max(alphas)])
plt.show()