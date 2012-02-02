function MultiRealTimeM(NumberOfChannels);
%MultiRealTimeM(NumberOfChannels) -- M-sequence visual receptive field
%mapping analysis program. NumberOfChannels = number of channels from
%which you wish to analyze data.

svr = actxserver ('ucdavis.msg.handler');

NumberOfSlides=16;

load('d:\\work\\AcuteRig\\MatlabScripts\\rfcolor.mat');
row=0;
for findex=1:NumberOfChannels
    fig(findex)=figure;
    for index=1:NumberOfSlides
        pHandles(findex,index)=subplot(ceil(sqrt(NumberOfSlides)),ceil(sqrt(NumberOfSlides)),index);   
        colormap(clay_color);
        image(zeros(16,16)+33);
        axis('square');
        axis off;
        title(['Frame ',num2str(index)]);   
    end
    set(fig(findex),'DoubleBuffer','on');
    %suptitle(['Channel ' num2str(findex)]);
    suptitle(['Channel ' char(64+findex)]);
    switch NumberOfChannels
        case {1, 2, 3, 4}
            FigureSize=round( (1024-70)/2);
            set(fig(findex),'Position',[1280+mod(findex-1,2)*FigureSize,(FigureSize+30) * ( 1 - floor((findex-1)/2) ) - 20,FigureSize,FigureSize]);
        case {5,6}
            FigureSize=round(1280/3);
            set(fig(findex),'Position',[1280+mod(findex-1,3)*FigureSize,(FigureSize+50) * ( 1 - floor((findex-1)/3) )  + 40 ,FigureSize,FigureSize]);     
        case {7,8}
            FigureSize=round(1280/4);
            set(fig(findex),'Position',[1280+mod(findex-1,4)*FigureSize,(FigureSize+60) * ( 1 - floor((findex-1)/4) )  + 40 ,FigureSize,FigureSize]);
    end        
end
    
Mseqfile='d:\\work\\AcuteRig\\MatlabScripts\\Big_M';
load(Mseqfile) %loads a 32767x256 M sequence. it's x256 to represent the msequence of each pixel

disp('Waiting for StartM message...')
msg='';

% djs Change strcmp to strncmp.
while (strncmp(msg, 'StartM', 6)~=1)
    %while (strcmp(msg,'StartM')~=1)
    msg=invoke(svr, 'getNextMessage');
    pause(.01)
end
disp('StartM message received: starting MSequence analysis');
disp(msg);

%extract channel numbers from message
chnums = ones(1,NumberOfChannels)*NaN;  %set up a variable to hold the actual channel numbers, fill with NaNs
if length(msg) > 7  %If there are appended channel numbers, they will come in positions 8:end
    msg2 = msg(8:end);  %cut channel numbers out of MSG
    tempcount = 0;
    while length(msg2) > 1
        tempcount = tempcount + 1;
        [temp msg2] = strtok(msg2);
        chnums(tempcount) = str2num(temp);
    end
    %and now just retitle the figures
    for findex=1:NumberOfChannels
        figure(findex); %call up current figure
        if ~isnan(chnums(findex))
            suptitle(['Channel ' num2str(chnums(findex))]);
        end
    end
end

msg='';
response=zeros(16,32767,8);
corr=zeros(16,256,8);
TermsProcessed=zeros(1,NumberOfChannels);
BlockSize=0;
ProcessAll=0;
Channel=0;
while(strcmp(msg,'StopM')~=1)
    %msg=NextMessage;
    msg=invoke(svr, 'getNextMessage');

    if strcmp(msg,'StopM')==1 return; end
    msg=str2num(msg);
    BlockSize=size(msg,2)-1;
    
    if (isempty(msg)==0)
        Channel=msg(1);
        if (TermsProcessed(Channel)+BlockSize) > 32767
            response(1,TermsProcessed(Channel)+1:32767,Channel)=response(1,TermsProcessed(Channel)+1:32767,Channel)+msg(2:32767-TermsProcessed(Channel)+1);
            response(1,1:BlockSize - (32767-TermsProcessed(Channel)),Channel)=response(1,1:BlockSize - (32767-TermsProcessed(Channel)),Channel)+msg(32767-TermsProcessed(Channel) +2:BlockSize+1);
        else
            response(1,TermsProcessed(Channel)+1:TermsProcessed(Channel)+BlockSize,Channel)=response(1,TermsProcessed(Channel)+1:TermsProcessed(Channel)+BlockSize,Channel)+msg(2:BlockSize+1);
        end
        for index=2:16
            response(index,:,Channel)=[response(index-1,2:32767,Channel) 0];
        end
        if TermsProcessed(Channel)+BlockSize-16 > 32767
            %disp(['Processing ' num2str(TermsProcessed(Channel)+1-16) ' to 32767']);
            corr(:,:,Channel)=corr(:,:,Channel)+response(:,TermsProcessed(Channel)+1-16:32767,Channel)*mseq(TermsProcessed(Channel)+1-16:32767,:);
            ProcessAll=1;
        elseif ( (TermsProcessed(Channel)-16 <=0) || (ProcessAll==1) )
            %disp(['Processing 1 to ' num2str(TermsProcessed(Channel)+BlockSize - 16)]);
            corr(:,:,Channel)=corr(:,:,Channel)+response(:,1:TermsProcessed(Channel)+BlockSize - 16,Channel)*mseq(1:TermsProcessed(Channel)+BlockSize - 16,:);
            ProcessAll=0;
        else
            %disp(['Processing ' num2str(TermsProcessed(Channel)+1 - 16) ' to ' num2str(TermsProcessed(Channel)+BlockSize - 16)]);
            corr(:,:,Channel)=corr(:,:,Channel)+response(:,TermsProcessed(Channel)+1 -16 :TermsProcessed(Channel)+BlockSize -16,Channel )*mseq(TermsProcessed(Channel)+1 -16:TermsProcessed(Channel)+BlockSize-16,:);
        end
        for index=1:16
            tempker(:,:,index)=(flipud(rot90(reshape(corr(index,:,Channel),16,16))));
        end
        %for {0,1} stimulus vector (Mseq): 
        Levsnew=.05;
        colormap(clay_color);
        ker=scaleimagetocolor(tempker - mean(mean(tempker(:,:,16))));
        figure(fig(Channel));
        for index=1:16
           colormap(clay_color);
           % axis('square');
           % axis off;
            set(get(pHandles(Channel,index),'Children'),'CData',ker(:,:,index));
        end
        drawnow;
 %       pause(.01);
        TermsProcessed(Channel)=mod(TermsProcessed(Channel)+BlockSize,32767);
    else
        pause(.01);
    end
    
end
disp('StopM received.');
return
