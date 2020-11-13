
import numpy as np
import matplotlib.pyplot as plt
from scipy.special import airy 

#an average characterstic oscillation period measured over the function's duration. (5 cycles between 0 and 15 seconds)
CHARPERIOD=3 #3 seconds

#the periods tested will be 10,100,250

#number of datapoints to be taken per period.
countinperiod=100

#uncertainty is assumed to be white noise (Gaussian), and is implemented in code as the normalized Gaussian distrubution with mean 0 and SD of 1
#uncertainty multipliers 
signaluncertaintyscale=.07
sensoruncertaintyscale=.025

#order of the polynomial fit used for global fits:
globalorder=15


#generate the actual function:
x = np.linspace(-5, 20, np.floor(25/CHARPERIOD*countinperiod).astype(int))
mask=np.logical_and(x<=16.8 , x>=0)
ai, _,_,_ = airy(x[mask]-12.8)
y=np.zeros(len(x))
y[mask]+=ai
ai=np.pad(ai,(100,100),'constant',constant_values=(0,0))

#as the investigator, you know the time t0 when you started and ended the pulse

#creating the corrupted noisy signal from the "true" data
noisey=(1+np.random.normal(0,1,y.shape)*signaluncertaintyscale)*y

#creating sensor measured signal
sensormeasured=noisey+np.random.normal(0,1,y.shape)*sensoruncertaintyscale


#plot the function, function corrupted by noise, and the sensor reading
rawdata=plt.figure(1)
plt.title("Raw Data with "+str(countinperiod)+" measurements/period")
plt.xlabel("Time")
plt.ylabel("Value")
plt.plot(x,sensormeasured,label='Actual Data Collected')
plt.plot(x,y,label='Ideal Signal')
# plt.plot(x,noisey,label='Perfect Measured Signal')
plt.legend()


def leastSquares(x,y, order):
    #this function takes in the data array, and maps an nth order polynomial fit to it
    arrlen=len(x)
    if order>arrlen-1:
        print("Fit order is too high for the data")
        return None
    else:
        #generate the polynomial fit array
        temparr=[]
        for i in range(order*2-1):
            temparr.append(np.sum(x**(i)))

        M=np.zeros((order,order))
        for i in range(order):
            for j in range(order):
                M[i,j]=temparr[i+j]
        
        #calcualate the equation vector

        ansvec=[]
        for i in range(order):
            ansvec.append(np.sum(x**i * y))
        ansvec=np.transpose(ansvec)
        try:
            coefs=np.matmul(np.linalg.inv(M),ansvec)
        except:
            coefs=np.zeros(len(ansvec))
        #coefficients are returned from 0th order to nth order
        return coefs

def polyPoints(xvals,coefs):
    #generates yvals from xvals
    yvals=np.zeros(xvals.shape)
    for i in range(len(coefs)):
        yvals+=coefs[i]*(xvals**i)
    return np.array(yvals)

def window(array, midpoint,width):
    #returns a window around a point
    if midpoint>=width and len(array)-midpoint-1>=width and midpoint>=0 and midpoint<=len(array):
        return array[midpoint-width:midpoint+width+1]
    else:
        print("invalid input to window function")
        return np.array()

def globalFit(x,y,order=globalorder,dataname='Fitted Data'):
    #global fit of all data
    globalpolyfit=leastSquares(x,y,order)
    largerx=np.linspace(min(x),max(x),1000)
    globalpolyvals=polyPoints(largerx,globalpolyfit)

    err=error(largerx,globalpolyvals,x,y)
    print(err)


    plt.figure()
    plt.title("Global fit with "+str(countinperiod)+" measurements/period \nError: "+str(err))
    plt.xlabel("Time")
    plt.ylabel("Value")
    plt.plot(x,y,label=dataname)
    plt.plot(largerx,globalpolyvals,label='Polynomial fit of order: '+str(order))
    plt.figtext(10,10,"Error: ")
    # plt.legend()

def windowedFit(x,y, width, overlap=0,dataname='Fitted Data',plot=True):
    #this calculates the windowed fit of the data.
    #width controls the width of the window (on either side of the point)
    #overlap controls how many points are overlapped between windows

    if overlap>2*width+1:
        print("Overlap exceeds width")
        return
    
    #number of windows/points that will be created
    #=number of datapoints - 2*1/2 width -
    #=floor(number of datapoints / (2*width+1-overlap))
    #note that this is going to be feedforward, so 
    windownum=np.floor((len(y)-overlap)/(2*width+1-overlap)).astype(int)

    xvals=[]
    yvals=[]
    plt.figure(figsize=(8,5.4))
    for i in range(windownum):
        tempx=window(x, width+i*(2*width+1-overlap) ,width)
        xvals.append(x[width+i*(2*width+1-overlap)])
        sliced=window(y, width+i*(2*width+1-overlap) ,width)
        #let the fit be 1 less order than the data for perfect data fitting.
        #3 is the max for numerical stability normally before shifting everything to the origin.
        #by shifting, the max numerical stability can be made much larger since the coefficients are relatively smaller

        shift=min(tempx)
        tempx-=shift
        coefs=leastSquares(tempx,sliced,len(sliced)-1)#len(sliced)-1
        tempx+=shift
        yvals.append(polyPoints(x[width+i*(2*width+1-overlap)]-shift, coefs))
        if plot:
            plt.plot(tempx,polyPoints(tempx-shift, coefs),'r')

    err=error(xvals,yvals,x,y)
    print(err)

    # plt.title("Piecewise fit, "+str(countinperiod)+" measurements/period \nSecond Order Fits\nError: "+str(err))
    
    plt.title("Piecewise fit, "+str(countinperiod)+" measurements/period \nWidth: "+str(width)+" Overlap: "+str(overlap)+"\nError: "+str(err))
    plt.xlabel("Time")
    plt.ylabel("Value")

    plt.plot(x,y,label="Actual Data")
    xvals=np.array(xvals)
    yvals=np.ravel(yvals)
    plt.plot(xvals,yvals,label="Fitted Curve")
    plt.legend()

    return np.array(xvals), np.ravel(yvals)

def optimalFit(x,y,seednum=23,stopcond=0.1,countstop=100,dataname='Optimal Fit',plot=True):
    #this calculates the "optimal" fit of the data.
    #optimal fit will be based off equalizing intra-class variance between different equally spaced "seed" nodes
    #somewhat like a modified version of an image processing algorithm.

    boundaries=np.floor(np.linspace(0,len(x),seednum+1)).astype(int)
    errors=np.zeros(seednum)
    count=0
    minerr=1000 #arbitrary large number
    finalxarr=[]
    finalyarr=[]

    while (np.max(errors)>=(1+stopcond)*np.min(errors) or count==0) and count<countstop:
        #Iterate until errors are homogenous or the stop count is met.
        count+=1
        tempxarr=[]
        tempyarr=[]
        for i in range(len(errors)):
            tempx=x[boundaries[i]:boundaries[i+1]]
            tempy=y[boundaries[i]:boundaries[i+1]]
            shift=min(tempx)
            tempx-=shift
            coefs=leastSquares(tempx,tempy,min(len(tempx)-1,6))#min(len(tempx)-1,6)
            tempx+=shift
            tempxarr.append(tempx)
            interpyvals=polyPoints(tempx-shift,coefs)
            tempyarr.append(interpyvals)

            errors[i]=np.sum((np.array(tempy)-interpyvals)**2)/len(tempy)

        if minerr>np.sum(errors).astype(float):
            minerr=np.sum(errors).astype(float)
            finalyarr=tempyarr
            finalxarr=tempxarr


        #using a gradient method to resize bounds.
        
        #the first and last boundaries are bookends and are not moved because they have to be at the ends of the function
        # print(len(boundaries))
        for i in range(1,len(errors)-1):
            # print(i)
            if errors[i+1]>errors[i] and boundaries[i+2]-boundaries[i+1]>1:
                boundaries[i+1]+=1
            elif errors[i]>errors[i+1] and boundaries[i+1]-boundaries[i]>1:
                boundaries[i+1]-=1

        #from the variations in the values, it's clear a nonlinear optimizer like simulated annealing might be needed to get better results than these gradient descent techniques.
        #this is because the assumption that smaller data ranges = less error is not always satisfied.
    
    err=error(np.concatenate(finalxarr).ravel(),np.concatenate(finalyarr).ravel(),x,y)
    print(err)

    if plot:
        plt.figure()
        plt.title("Optimal Fit with "+str(countinperiod)+" measurements/period\nError: "+str(err))
        plt.plot(x,y)
        for xvals,yvals in zip(finalxarr,finalyarr):
            plt.plot(xvals,yvals,'r')
            plt.plot(xvals[0],yvals[0],'g*')
            plt.plot(xvals[-1],yvals[-1],'g*')
        plt.legend(["Data","Optimal Fit"])

def error(series1x, series1y, series2x,series2y):
    #calculate the error using normalized RMS value with interpolated points
    if len(series1x)>len(series2x):
        #must interpolate on series2x
        comparitor=np.interp(series1x,series2x,series2y)
        return np.sqrt(np.sum((comparitor-series1y)**2)/len(comparitor))
    else:
        #must interpolate on series1x
        comparitor=np.interp(series2x,series1x,series1y)
        return np.sqrt(np.sum((comparitor-series2y)**2)/len(comparitor))






#Code begins here (same as if name=="_main_")

globalFit(x,y,dataname="Noiseless Data")

xvals,yvals=windowedFit(x,sensormeasured, 5, overlap=1,plot=False)

optimalFit(x,y,seednum=30)
optimalFit(x,sensormeasured,seednum=40,countstop=50)

plt.show()

