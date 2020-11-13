function hydrophonecorrtest
close all
[y,Fs] = audioread("stereo_2017-02-5_09-41-11.wav");

%Use this to see a single pulse only
y=y(1:40000,:);

figure
plot(y)

disp("Estimating time delay tau with linear correlations")

corr=xcorr(y(:,1),y(:,2));

%'MinPeakDistance' does not seem to help
[pks,locs]=findpeaks(corr);
figure
plot(corr)
hold on

%using knowledge of the pinger pinging every 2 seconds (50000 tics):

%Wow this is so inneficient.  Better would be to integrate such checking into the original peak finding
%this may not be an issue, as I don't believe we will have enough memory to
%record 2 seconds at the rate we sample at.
[pks,locs]=corrmax(pks,locs);

plot(locs,pks,'x')


%let's just say, for this example, we have no issues with receiving
%multiple pings, so the time delay is the first one

samplingrate=50000/2; %samples/ 2 seconds (in practice, use the known sampling rate)
samplingrate=96000

%the linear correlation of the signals of the same length effectively
%multiplies the length of the signal by 2.  To get the 0 point of the
%signals, you need to divide them by 2
tau1=(locs(1)-length(corr)/2) /samplingrate %converting to seconds)


%The hyperbolic formulation is simply the subtraction of one time formula
%from another.
%Like one would expect, you need n+1 hydrophones in which no 3 hydrophones are colinear to uniquely determine a 2D

c=1484; %m/s speed of sound in water
x=sym('x');
y=sym('y');
%constraint equation.  Required for this situation with 2 hydrophones on a
%wall, where we know the second solution is not even in the water
assume(y>0); %it doesn't look like this does anything.  The solver gives y values less than 0
assume(x,'real')
assume(y,'real')

xa=.5/2; %m
xb=-xa; 

ya=0;
yb=ya;

solution=solve(tau1==1/c*(sqrt((x-xa)^2+(y-ya)^2)-sqrt((x-xb)^2+(y-yb)^2)),[x,y])
solution.x
solution.y

1/c*(sqrt((solution.x(3)-xa)^2+(solution.y(3)-ya)^2)-sqrt((solution.x(3)-xb)^2+(solution.y(3)-yb)^2))
%The solution to this problem is (-1.0466, 2.5536), indicating the buoy is
%2.5536 meters forward, and -1.0466 meters to the right (so it's to the
%left as we expect)

f=@(x,y) -tau1+1/c*(sqrt((x-xa)^2+(y-ya)^2)-sqrt((x-xb)^2+(y-yb)^2))
figure
contourplot=fcontour(f, [-10 10  -10 10]);
contour(contourplot.XData,contourplot.YData,contourplot.ZData,'ShowText','on')

%given the underdefined situation, both x and y cannot be uniquely solved
%without a third hydrophone

%using the parallel-arrival assumption (good for long distances)
theta=asind(c*tau1/(xa-xb))


function [outpeaks,outlocs]=corrmax(peaks,locs)
    outpeaks=[];
    outlocs=[];
    %note that 3/4 is used to create an overlap between expected windows.
    %the theoretical absolute minum is 1/2 with marginal overlap.
    width=floor(50000*3/4);

    %one could also order the peaks largest to smallest , then remove any
    %peaks that were within a width less than 2 pulse widths away, until no
    %peaks remain.  This would cut down the bins far faster.   

    for i=1:length(pks)
        if peaks(i)==max(peaks(locs<=locs(i)+width & locs>=locs(i)-width))
           outpeaks=[outpeaks,peaks(i)];
           outlocs=[outlocs,locs(i)];
           %faster for a lower-level algorithm would be to remove spurious
           %local maxima from consideration to keep them from being
           %evaluated agin (see above)
        end
    end
end


end
