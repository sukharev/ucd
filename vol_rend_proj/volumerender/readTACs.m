function readTACs(filename)
%This m-file is a short example of how to read and write a binary file
%in MATLAB using low level routines

%filename = 'VolData_time0_10_x142_167_y26_33_z18_18.cfg';

%read in VolumeData binary file
fid = fopen(filename,'rb')  %open file
mat = fread(fid, [8 1], 'int32');  %read in the header
xsize = mat(2) - mat(1)+1;
ysize = mat(4) - mat(3)+1;
zsize = mat(6) - mat(5)+1;

time_in = mat(7)+1;
time_out = mat(8);

i = time_in;
setsize = ysize*zsize*xsize;
% TODO: get number of timesteps from the file
%      get the size of the original dataset from the file
while i <= time_out
    timestep = fread(fid, [1], 'int32');  %read in the header
    timestep
    temp = fread(fid, [setsize], 'float');  %read in the data
    voldata(i,:) = temp';
    i = i + 1;
end
fclose(fid)  %close file

for k=1:1:10
    n = 1;
    for x=1:1:xsize
        for y=1:1:ysize
            for z=1:1:zsize
                if zsize == 1
                    D(x,y) = voldata(k,n);
                elseif xsize == 1
                    D(y,z) = voldata(k,n);
                elseif ysize == 1
                    D(x,z) = voldata(k,n);
                end
                n = n+1;
            end
        end
    end
%    load flujet
%    image(D);
%    colormap(jet)
    %colormap(map);
    %K = mat2gray(D);
    DS = imresize(D, [400 400]);
    DS = rot90(DS);
    imshow(DS)%, figure %imshow(K)
    %pic = rgb2ind(D,185);
    %image(pic);
    %colorbar;
   
%    J = rgb2gray(pic);
%    figure, imshow(pic), figure, imshow(J)

 
end