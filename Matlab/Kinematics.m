% RPYrotate(1,0,0)
% 
% % vec=[0,1,0]
% % rotateZ(rotateY(rotateX(vec,0),1),0)
% theta=1;
% z=[cos(theta),-sin(theta),0;
%  sin(theta),cos(theta),0;
%  0,0,1];
% theta=0;
% y=[cos(theta),0,sin(theta);
%         0,1,0;
%         -sin(theta),0,cos(theta)];
% theta=0;
% x=[1,0,0;
%         0, cos(theta),-sin(theta);
%         0,sin(theta),cos(theta)];
% z*y*x
% R={z,y,x}
% propogateRotations(R)
% 
% asdf=rotateAboutAxis([1,3,-2],.232)
% p=[1,1,1,0,1]
% R={z,asdf,x,y,asdf}
% origins={[0,0,0],[0,0,1],[0,2,3],[1,2,-8],[0,2,0]}
% revoluteJacobian(p,R)  
% translationalJacobian(p,R,origins)
% 
% analyticalJacobian(p,R,origins)



function [rotarray]=rotateZ(theta)
    %rotates about the z axis by angle theta
    rotarray=[
        cos(theta),-sin(theta),0;
        sin(theta),cos(theta),0;
        0,0,1];
end

function [rotarray]=rotateY(theta)
    %rotates about the y axis by angle theta
    rotarray=[
        cos(theta),0,sin(theta);
        0,1,0;
        -sin(theta),0,cos(theta)];
end

function [rotarray]=rotateX(theta)
    %rotates about the x axis by angle theta
   rotarray=[
        1,0,0;
        0, cos(theta),-sin(theta);
        0,sin(theta),cos(theta)];
end

function[rotarr]=rotateAboutAxis(axis,theta)
    %axis is a length 3 axis which represents a vector axis from the origin
    %theta is the radian rotation about this axis
    %outputs the rotation matrix 
    
    c=cos(theta);
    s=sin(theta);
    v=1-c;
    
    kx=axis(1);
    ky=axis(2);
    kz=axis(3);
        
    rotarr=[kx^2*v+c, kx*ky*v-kz*s, kx*kz*v+ky*s;
            kx*ky*v+kz*s, ky^2*v+c, ky*kz*v-kx*s;
            kx*kz*v-ky*s, ky*kz*v+kx*s, kz^2*v+c];
    
end

function [rotarr]=eulerrotate(phi, theta, psi)
    %The euler angle representation of a frame
    %phi is the first rotation about z-axis, theta is second rotation about
    %y-axis, psi is the third rotation about z-axis again
    rotarr=[
        cos(phi)*cos(theta)*cos(psi)-sin(phi)*sin(psi), -cos(phi)*cos(theta)*sin(psi)-sin(phi)*cos(psi), cos(phi)*sin(theta);
        sin(phi)*cos(theta)*cos(psi)+cos(phi)*sin(psi), -sin(phi)*cos(theta)*sin(psi)+cos(phi)*cos(psi), sin(phi)*sin(theta);
        -sin(theta)*cos(psi), sin(theta)*sin(psi), cos(theta)];
    
    %the basis vectors are column-wise, so from left to right, each vector
    %is the 3-numbers downwards
end

function [phi,theta,psi]=inverseeulerrotate(matrix)
    %finds the euler angles of a rotation matrix
    theta=acos(matrix(3,3));
    if sin(theta)>0
        phi=atan2(matrix(2,3),matrix(1,3));
        psi=atan2(matrix(3,2),-matrix(3,1));
    elseif sin(theta)<0
        phi=atan2(-matrix(2,3),-matrix(1,3));
        psi=atan2(-matrix(3,2),matrix(3,1));
    else
        %due to gimbal lock, if there is no nonzero theta, there is
        %dependence between the psi and phi rotation angle, requiring this
        %third borderline case.  phi is 0 by convention
        psi=acos(matrix(1,1))
        phi=0
    end
end

function [rotarr]=RPYrotate(roll,pitch,yaw)
    %first a yaw about x (psi)
    %second a pitch about y (theta)
    %last a roll about z (phi)
    
    %NOTE.  THIS FORMULA IS INCORRECT IN THE TEXTBOOK.  IT DOESN'T EQUAL
    %THE EXPECTED ROTATIONS WHEN MANUALLY DOING ZYX MULTIPLICATION
    %seems like the bottom row is wrong in this formulation
    rotarr=[cos(roll)*cos(pitch), -sin(roll)*cos(yaw)+cos(roll)*sin(pitch)*sin(yaw),sin(roll)*sin(yaw)+cos(roll)*sin(pitch)*cos(yaw);
            sin(roll)*cos(pitch), cos(roll)*cos(yaw)+sin(roll)*sin(pitch)*sin(yaw),-cos(roll)*sin(yaw)+sin(roll)*sin(pitch)*cos(yaw);
            -sin(roll),cos(roll)*sin(yaw),cos(roll)*cos(yaw)];
end

%note that there is an inverse way to find roll pitch and yaw from a
%matrix, but I have neglected its inverse as it is not directly listed in
%the text.

function [homomatrix]=homoMatrix(rotmatrix,translationvec)
    %computes a homogeneous matrix from a rotation matrix and translation
    %vector
    homomatrix=eye(4);
    homomatrix(1:3,1:3)=rotmatrix;
    homomatrix(1:3,4)=translationvec;
end

function [invhomomatrix]=invHomoMatrix(homomatrix)
    %computes the inverse of a homogeneous matrix
    %strangely enough, this is actually slower than matlab's inv() function
    %by a factor of 2
    invhomomatrix=eye(4);
    invhomomatrix(1:3,1:3)=transpose(homomatrix(1:3,1:3));
    invhomomatrix(1:3,4)=-transpose(homomatrix(1:3,1:3))*homomatrix(1:3,4);
end

%like the method below, it is possible to propogate homogeneous transforms
%by multiplying them by themselves continuiously

function [outmatrix]=DHFrame(a,d,alpha, theta)
   %Denavit Hartenberg representation of a matrix
   
   %see section 3.2.3 in spong to see the rules that you need to follow to
   %use this
   
   %to find the final position at any end-effector, multiply these matrices
   %together in the order which they appear from the root rigid frame
   outmatrix=[
       cos(theta),-sin(theta)*cos(alpha), sin(theta)*sin(alpha), a*cos(theta);
       sin(theta),cos(theta)*cos(alpha), -cos(theta)*sin(alpha), a*sin(theta);
       0,sin(alpha), cos(alpha), d;
       0,0,0,1]
end

function [finalmatrix]=propogateDHFrame(a,d,alpha,theta)
    %forward propogates/multiplies using the Denavit Hartenberg convention
    %all inputs should be vector valued and cell arrays
    
    finalmatrix=DHFrame(cell2mat(a(1)),cell2mat(d(1)),cell2mat(alpha(1)), cell2mat(theta(1)))
    for i=2:length(a)
        newframe=DHFrame(cell2mat(a(i)),cell2mat(d(i)),cell2mat(alpha(i)), cell2mat(theta(i)))
        finalmatrix=finalmatrix*newframe
    end
end

function [finalmatrix]=propogateRotations(R)
    %Multiplies Rotations together to find a final rotation matrix
    %R is a cell array of rotation matrices
    finalmatrix=eye(3);
    for i=1:length(R)
        finalmatrix=finalmatrix*cell2mat(R(i));
    end
end



%Jacobians and Velocities-------------------------------

%Velocities can always be represented with a skew matrix of a vector of
%rotation w

function [S]=skewmatrix(w)
    x=w(1);
    y=w(2);
    z=w(3);
    S=[0,-z,y;
       z,0,-x;
       -y,x,0;];
    %multiplication by a skew matrix is the equivalent of a cross product
end

%functions for calculating rotations

function [rdot]=rotationVel(omegas, rotmatrices)
    %omegas is a list of rotation vectors, rotmatrices is a list of
    %rotation matrices
    %omegas and rotmatrices are cell arrays
    
    %Eqn. 5.23
    w=calculatew(omegas, rotmatrices)
    S=skewmatrix(w)
    R=propogateRotations(rotmatrices)
    rdot=S*R
end

function [w]=calculatew(omegas, rotmatrices)
   %omegas is list of omegas between frames, starting from the base fixed frame, and moving concecutively down the chain of frames
   %omegas are a series of length 3 vectors, of rotations about x, y, and z
   %axis that the position of the next frame is represented in
   
   %rotmatrices is the corresponding rotation matrices, length 1 less than
   %omegas due to there being an identity rotation matrix for the first
   %omega
   
   %eq 5.36
   
   w=cell2mat(omegas(1))
   for ind=1:length(rotmatrices)
       w=w+cell2mat(rotmatrices(ind))*cell2mat(omegas(ind))
   end
end

    

function [Jw]=revoluteJacobian(p,R)    
    %p is the prismatic vector.  Is true/1 valued if the joint is revolute
    %and False/0 otherwise
    
    %R is the vector of matrix transformations, of length 1 less than p,
    %given that the first element is the identity due to it being the
    %transformation from fixed frame. cell array
    
    
    %note that the z vector of 3D vectors contains the information about
    %the axis of revolution for each frame with respect to the frame before
    %it
    
    %Jw is a 3xn matrix
    
    z=[0;0;1];
    Jw=[p(1).*z];
    for i=2:length(p)
        %note that you could also use one of the above methods to propogate
        %the transformations down the line of bodies
        
        %the rotation matrix we want is the rotation of a frame with
        %respect to the origin frame, and proceeds by multiplying rotations
        %by each other
        
        lastR=cell2mat(R(i-1));
        z=lastR*z;
        
        Jw=[Jw,p(i).*z];
    end
end
   
function [Jv]=translationalJacobian(p,R,origins)
    %p is the prismatic vector.  Is true/1 valued if the joint is revolute
    %and False/0 otherwise
    
    %R is the vector of matrix transformations, of length 1 less than p,
    %given that the first element is the identity due to it being the
    %transformation from fixed frame.  cell array
    
    %origins is the xyz location of the set of origins for the frames
    %note that this script can be modified with lengths of elements solving
    %for end positions at each rotation matrix. cell array
    
    
    %note that the z vector of 3D vectors contains the information about
    %the axis of revolution for each frame with respect to the frame before
    %it
    
    %Jv is a 3xn matrix
    
    %for ease of use, add the identity matrix to the front of R
    
    Jv=[];
    R=[eye(3),R];
    z=[0;0;1];
    for i=1:length(p)
        z=cell2mat(R(i))*z;
        if p(i)==0
            %joint is prismatic and therefore not rotating
            Jv=[Jv,z];       
        else
            %joint is revolute and rotates
            %the subtraction of elements means that the final frame (i=n) will
            %neccisarily have a 0 component, as the vector from itself to
            %itself has length 0
            Jv=[Jv,transpose(cross(z,cell2mat(origins(end))-cell2mat(origins(i))))];
        end
    end
end

function [J]=geometricJacobian(p,R,origins)
    %[v;w]=J*qdot
    %qdot is the derivative of joint variables
    
    %for q:
    %If joint i is prismatic, then thetai is also a constant, while di
    %is the ith joint variable. Similarly, if joint i is revolute, then di is constant and i is the ith
    %joint variable.

    Jv=translationalJacobian(p,R,origins);
    Jw=revoluteJacobian(p,R);
    J=[Jv;
       Jw];
end
    

function [Ja]=analyticalJacobian(p,R,origins)
    %eulerangles is the euler angles between base and final frame

    %the analytical jacobian is the jacobian that transforms the qdot
    %vector into a xdot vector comprised of a velocity vector and an
    %angular velocity vector with respect to an initial fixed frame
    
    %this angular basis is euler angles
    
    %it can be derived from the geometric jacobian
    
    %5.114
    
    finalR=propogateRotations(R);
    
    %eulerangles of final frame relative to origin frame
    [phi,theta,psi]=inverseeulerrotate(finalR);
    
    
    T=[cos(psi)*sin(theta), -sin(psi), 0;
       sin(psi)*sin(theta), cos(psi), 0;
       cos(theta), 0,1];
    if det(T)==0
        disp('Cannot Compute, the Euler Angles are Singular')
    else   
        J=geometricJacobian(p,R,origins);
        Ja=[eye(3),zeros(3);
            zeros(3),inv(T)]*J;
    end
    
end


%singularities are configurations where there can be an unbounded output
%from a bounded input, or complete inability to have an output with any
%given input
%they occur when the determinant of the jacobian is zero and the matrix is
%singular


% function [A]=acceleration(p,R,origins)
%     %[a;alpha]=A*qdoubledot
%     %qdoubledot is the second derivative of joint variables
%     
%     J=geometricJacobian(p,R,origins)
%     J*
%     
% end


