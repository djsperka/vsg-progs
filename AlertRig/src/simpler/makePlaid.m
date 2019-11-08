function [I] = makePlaid(W, H, p, sf0, x0, y0, alpha0, sf1, x1, y1, alpha1)

I = zeros(H, W);
k0 = 2 * pi * sf0 / p;
k1 = 2 * pi * sf1 / p;

for x=1:W
    for y=1:H
        phase0 = k0*((x-x0)*cos(2*pi*alpha0/360) + (y-y0)*sin(2*pi*alpha0/360));
        phase1 = k1*((x-x1)*cos(2*pi*alpha1/360) + (y-y1)*sin(2*pi*alpha1/360));
        I(x,y) = cos(phase0) + cos(phase1);
    end
end
return;
end
    
