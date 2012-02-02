function outmatrix = scaleimagetocolor(inmatrix)
% outmatrix = scaleimagetocolor(inmatrix): scale matrix to colormap.
% inputs: one frame formatted in a matrix
% outputs: image matrix scaled to colormap
% Based on scaleimagetocolorSY.m
% Usrey Lab 3.26.01 /jm

% inmatrix
maxmax = max(max(max(inmatrix)));
minmin = min(min(min(inmatrix)));
scaleregion = max([abs(minmin),abs(maxmax)]);

% if none of the pixel responses are > threshold, inmatrix will be zeros.
if scaleregion == 0
    scaleregion = 1;
end

cmap = colormap;
sz = size(cmap);
scale = 0.5*(sz(1) - 1)/scaleregion;

% output
outmatrix = round(inmatrix*scale + 0.5*(sz(1) - 1));
