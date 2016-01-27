Theta=30*pi/180;
R=[1 0 0;0 cos(Theta) sin(Theta);0 -sin(Theta) cos(Theta)];
t=[0 0 -10]';
M=[R,t];

fovy=30*pi/180;
f=1/tan(fovy/2);
P=[.5 0 .1;0 .5 .2;0 0 1];
eyes=[-1 -1 1 1; -1 1 -1 1; 0 0 0 0; 1 1 1 1];

peyes=P*M*eyes;
peyesz=peyes(1:2,:)./peyes(3:3,:);
An=[];
for ei = 1:size(eyes,2);
	eyoh=[eyes(1:2,ei)',1];
	py=peyesz(1:2,ei)';
	a=P(1,1);
	b=P(2,2);
	u0=P(1,3);
	v0=P(2,3);
	s=P(1,2);
	An=[An;a.*eyoh,s*eyoh,(u0-py(1))*eyoh;0.0*eyoh,b.*eyoh,(v0-py(2))*eyoh];
end

so=null(An);

nR=zeros(3);
nR(:,1)=so([1,4,7]);
nR(:,2)=so([2,5,8]);
nt=so([3,6,9]);

#mag2(nR1)+mag2(nR2)+mag2(t) = 2+x where x is some scalar.
#BUT null-space solves it under the constraint that mag2(nR1)+mag2(nR2)+mag2(t)=1.
#so we're looking for some scalar s s.t. s*(mag2(nR1)+mag2(nR2))=2

z=(norm(nR(:,1))+norm(nR(:,2)))/2; #average magnitude of the r vectors is what we are really computing there
nt=nt/z;
nR=nR/z;
nR(:,3)=cross(nR(:,2),nR(:,1)); #enforce right hand coordinate system.
nM=[nR,nt] * det(nR); #enforce right hand coordinate system
