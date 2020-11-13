'''
Created on May 30, 2019

@author: Daniel Foreman
'''

import numpy as np
import matplotlib.pyplot as plt
import scipy.stats as stats

def MakeRange(estimate, stderr, num_stderrs,num_points=100):
    spread = stderr * num_stderrs
    array = np.linspace(estimate-spread,estimate+spread,num_points)
    return array

def FindPriorRanges(xs, num_stderrs=3.0):
    # compute m and s
    n = len(xs)
    m = np.mean(xs)
    s = np.std(xs)
    # compute ranges for m and s
    stderr_m = s / np.sqrt(n)
    mus = MakeRange(m, stderr_m, num_stderrs)
    stderr_s = s / np.sqrt(2 * (n-1))
    sigmas = MakeRange(s, stderr_s, num_stderrs)
    return mus, sigmas

def Likelihood(x,mus,sigmas):
    #like = stats.norm.pdf(x, mu, sigma) #due to it looking like a band, I'm thinking this doesn't actually do the outer product, and only iterates on 1 dimension (which is mu) I think there might be no way around it but I'll have to write a general method or use a general method to iterate a point function over a n-dimensional dataset
    #right now, this takes a single value and is iterated below in a for loop.  Most efficient is simply taking the entire set of observed x, and using it as a dimension with length greater than 1 to make the whole array in 1 command
    like=np.nan_to_num(np.fromfunction(lambda x,a,b:stats.norm.pdf(x,mus[a],sigmas[b]), (len(x),len(mus),len(sigmas)),dtype=np.int))#stats.norm.pdf(x,mu,sigma)
    #like=np.nan_to_num(np.fromfunction(lambda a,b,x=x:stats.norm.pdf(x,mus[a],sigmas[b]),(len(x),len(mus),len(sigmas)),dtype=np.int))
    return like

def logLikelyhood(x,mu,sigma):
    like=Likelihood(x, mu, sigma)
    largest=np.max(like)
    return np.nan_to_num(np.log(like/largest))
    #his code also has something to remove 0 probability elements to prevent -Inf when you apply log

def expLogLikelyhood(logdist):
    m=np.max(logdist)
    return np.exp(logdist-m)

#def mulikelyhood(m,mu,sigma):
    
    

def approxBayesianComputation(data,mus,sigmas):
    n=len(data)
    m=np.mean(data)
    s=np.std(data)
    #he adds together the likelyhoods (log) of the mean and the sigma occuring given the particular dataset (rather than the exact data we got here (in terms of numbers))

    #would be two seperate apply along axes
    #muloglike=np.apply_along_axis(func1d, 0, arr)
    
    #very slow
    
#     arr=np.ones([len(mus),len(sigmas)])
#     for muind,mu in enumerate(mus):
#         for sigmaind,sigma in enumerate(sigmas):
#             stderr_m = sigma / np.sqrt(n)
#             loglike=np.log(stats.norm.pdf(m, mu, stderr_m))
#             stderr_s=sigma/np.sqrt(2 * (n-1))
#             loglike+=np.log(stats.norm.pdf(s, sigma, stderr_s))
#             arr[muind][sigmaind]=loglike
# 
#     return np.exp(arr)    

    #without log likeyhood
    arr=np.ones([len(mus),len(sigmas)])
    for muind,mu in enumerate(mus):
        for sigmaind,sigma in enumerate(sigmas):
            stderr_m = sigma / np.sqrt(n)
            loglike=stats.norm.pdf(m, mu, stderr_m)
            stderr_s=sigma/np.sqrt(2 * (n-1))
            loglike*=stats.norm.pdf(s, sigma, stderr_s)
            arr[muind][sigmaind]=loglike

    return arr      

    #both distributions appear to have that right skew/comet tail, although it doesn't appear that the original distribution had this (although this is exactly his code)
        
def Normalize(arr):
    return arr/np.sum(arr)

#data=[1,2,3,4,5,6,7,8,2,3,4,5,6,7,3,3,4,4,1,1,1,1,1,1,1,1,1,1,1,1,12,2]*70
#data=[1,4,2,5,6,7,8,-3,-4,-5,-1]
#data=[1,1,1,1,1,1,2,2,2,0,0,0]
data=[1,2,3,4,6]
#data=[-2,0,2,3,4,5]
#data=[7,7,7,7,7,7,7,7,7,7,7,7,7.1]
#data=[0,-1,1,-2,2,-3,3,-4,4,-5,5,-6,6]
#data=[0,0,0,0]
#data=[5,5,5,5]

#all likely ranges for mu and sigma (seems like this doesn't work well all the time for some reason)
mus, sigmas=FindPriorRanges(data)

#sigmas=np.linspace(.01,10,100)
#mus=np.linspace(0,30,1000)

prior=np.ones([len(mus),len(sigmas)])

#make 2d distribution

multarray=Likelihood(data,mus,sigmas)

# multarray=[]
# for dataelem in data:
#     multarray.append(Likelihood(dataelem,mus,sigmas))

posterior=Normalize(np.multiply(prior,np.prod(multarray,axis=0)))

plt.imshow(posterior, cmap='gist_heat', interpolation='nearest',extent=[min(sigmas),max(sigmas),min(mus),max(mus)])#extent=[min(mus),max(mus),min(sigmas),max(sigmas)]
plt.show()

coefofvariation=np.outer(mus,1/sigmas)
plt.imshow(coefofvariation, cmap='winter', interpolation='nearest')
plt.show()


#using log likelyhoods for larger datasets (note that this works on large datasets where the above fails due to discretization error

# multarray=[]
# for dataelem in data:
#     multarray.append(logLikelyhood(dataelem,mus,sigmas))

multarray=logLikelyhood(data,mus,sigmas)
#the product of logs is the log of the sum
posterior=Normalize(np.multiply(prior,expLogLikelyhood(np.sum(multarray,axis=0))))

plt.imshow(posterior, cmap='gist_heat', interpolation='nearest',extent=[min(sigmas),max(sigmas),min(mus),max(mus)])
plt.show()

plt.imshow(approxBayesianComputation(data, mus, sigmas), cmap='gist_heat', interpolation='nearest',extent=[min(sigmas),max(sigmas),min(mus),max(mus)])
plt.show()

#to make faster, see apply_over_axes test
#the other methods now use the super fast way

