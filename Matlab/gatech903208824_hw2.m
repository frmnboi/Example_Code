% Script: Numerical Approximations
%
% This script runs 5 different numerical approximations of step size
% 1,2,5,10, and 20 of the differential equation modeling the velocity of a
% parachutist.  Each numerical approximation is given an error percentage,
% which is the approximation's greatest deviation from the actual value.
%
%
% Outputs:
% Plot of actual vs numerical approximations of the air resistance problem
% Plots the max percentage error of the numerical approximations
%
% Author: Daniel Foreman
% Section: ME 2016 - C
% Assignment: 2 
% Date: 1/25/2017

function gatech903208824_hw2

FDD_parachutist; %run the FDD subfunction
end





% This function plots analytical and numerical solutions to the air
% resistence problem as well as the maximum error of each of the numerical
% solutions
%
% Inputs:
% N/a
%
% Outputs:
% 1 plot of the numerical solutions plus the analytical solution
% 1 plot of the maximum error of the numerical approximations
%
% Author: Daniel Foreman
% Section: ME 2016 - C
% Assignment: 2 
% Date: 1/25/2017

function FDD_parachutist
close all;
%% define constants
c = 12.5;% [kg/s]
m=75;% [kg] 75
g=10; %m/s^2]
t0 = 0; %[s]
tend = 60; %[s]
step=1; % dt in [s]
h=[1,2,5,10,20]; %step size for numerical solution
color=['g','b','k','y','m']; %color of the plotted lines
%% analytical solution
t=t0:tend; %generate time values to run through the functions
u=g*m/c*(1-exp(-(c/m)*t));%generate analytical solution values

%plot analytical solution
figure
plot(t, u, 'r')
ylabel('Time(S)','FontWeight','bold')
xlabel('Velocity v(t)','FontWeight','bold')
hold on


%% numerical solution
for i=1:length(h)
%use the FDD function to numerically approximate the values of the velocity
%using different step sizes
[velocity]=fdd(step,tend,g,c,m,h(i));

%plot numerical solution with different colors
plot(velocity(:,1), velocity(:,2),['--',color(i)])


%Calculate the error of each of the series' final values
errors(i)=calculateError(velocity, [t',u'])


end

%add plot title and legend to the numerical vs analytical graph
title('Parachutist Example, Terminal Velocity: Exact Vs Numerical','FontWeight','bold')
legend(['Exact        ';'Numer Step 1 ';'Numer Step 2 ';'Numer Step 5 ';'Numer Step 10';'Numer Step 20'])
   
%start making the relative error vs step size graph
figure
plot(h,errors,'-o');
title('Step Size vs. Relative Error');
xlabel('Step Size');
ylabel('Relative Error (%)');


end

% This function calculates the velocity via a recursive numerical method
%
% Inputs:
% 1-unused
% 2-the final time point
% 3-gravitational constant
% 4-air resistance constant
% 5-object mass
% 6-step size
%
% Outputs:
% The velocities at each time point in a numerical estimation of the
% velocity of a falling parachutist
%
% Author: Daniel Foreman
% Section: ME 2016 - C
% Assignment: 2 
% Date: 1/25/2017

function [velocity]=fdd(step,tend,g,c,m,h)
        % finite divided difference approximation
        
        velocity=[0,0];
        v=0;
        for i=1:h:tend%start for loop from 1
            v=v+(g-c/m*v)*h;
            velocity=[velocity;[i,v]];
        end
end



% This function calculates the maximum error of a single numerical series
% relative to a known true analytical value
%
% Inputs:
% 1-the velocities and associated times of the numerical solutions
% 2-the velocities and associated times of the analytical solution
%
% Outputs:
% The maximum error of the numerical solution.
%
% Author: Daniel Foreman
% Section: ME 2016 - C
% Assignment: 2 
% Date: 1/25/2017
function [error]=calculateError(velocity, trueval)
%velocity and trueval are arranged like: [time, velocity;...]
[r,~]=size(velocity);
error=[];
%find error values for each numerical value (the number of steps must be a
%multiple of the total number of data points used in the analytical
%approximation)
for i=1:r
error(i)=abs(trueval(trueval(:,1)==velocity(i,1),2)-velocity(i,2))./trueval(trueval(:,1)==velocity(i,1),2);
end
%generate the error in percentages
error=max(error)*100;
end