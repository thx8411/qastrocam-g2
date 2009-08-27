#include "fits.hpp"
#include <stdio.h>

#include <string>

Fits::Fits() {
   x_=y_=-1;
   dataType_=notDefined;
   data_=NULL;
}

void Fits::connectData(int width, int height,
                       dataType type, const void * data) {
   x_=width;
   y_=height;
   dataType_=type;
   data_=data;
   // init of basic properties
   properties_.clear();
   setProperty("SIMPLE",true);
   switch (dataType_) {
   case notSigned8bit:
      setProperty("BITPIX",8);
      break;
   case signed16bit:
      setProperty("BITPIX",16);
      break;
   case signed32bit:
      setProperty("BITPIX",32);
      break;
   }
   setProperty("NAXIS",2);
   setProperty("NAXIS1",x_);
   setProperty("NAXIS2",y_);
}

void Fits::setProperty(string name,bool value) {
   char buff[81];
   snprintf(buff,81,"%s=                    %c",
            padTo8(name).c_str(),
            value?'T':'F');
   properties_.push_back(padTo80(buff));
}

void Fits::setProperty(string name,int value) {
   char buff[81];
   snprintf(buff,81,"%s=          %10d",
            padTo8(name).c_str(),
            value);
   properties_.push_back(padTo80(buff));
}

void Fits::setProperty(string name,string value) {
   if (value.size() < 8) {
      value+="        ";
   }
   char buff[81];
   snprintf(buff,81,"%s= '%s'",
            padTo8(name).c_str(),
            value.c_str());
   properties_.push_back(padTo80(buff));
}

string Fits::padTo80(string str) {
   char buff[81];
   strncpy(buff,str.c_str(),80);
   buff[80]=0;
   for (int i=str.size();i<80;++i) {
      buff[i]=' ';
   }
   return buff;
}

string Fits::padTo8(string str) {
   char buff[9];
   strncpy(buff,str.c_str(),8);
   buff[8]=0;
   for (int i=str.size();i<8;++i) {
      buff[i]=' ';
   }
   return buff;
}

bool Fits::write(const string & fileName) const {
   FILE * file;
   file=fopen(fileName.c_str(),"wb");
   if (file == NULL) {
      return false;
   }
   for(list<string>::const_iterator propIt=properties_.begin();
       propIt!=properties_.end();
       ++propIt) {
      fprintf(file,propIt->c_str());
   }
   // end final
   fpos_t pos;
   fprintf(file,"END");
   fgetpos(file,&pos);
   while(pos.__pos<2880) {
      fprintf(file," ");
      fgetpos(file,&pos);
   }
   return true;
}

/**************

unit ufits;

interface

{uses Registry,Math,
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  StdCtrls, ExtCtrls, Grids, Outline, DirOutln, FileCtrl, Buttons, ComCtrls,
  Menus, Mask, ToolEdit, SystemTreeView;}

uses Windows, Messages, SysUtils, Controls, Dialogs, FileCtrl, uParamFITS, FastDIB;

type
FITSheader = array [1..36,1..80] of char;
FITSrec16 = array [1..1440] of smallint ;
FITSrec8 = array [1..2880] of byte ;
TTypeFITS = (fitRVB,fitR,fitV,fitB);

Procedure WriteFITS(p_nom,p_commentaire : String; p_typfit : TTypeFITS; p_bmp : TFastDIB; var pv_ok : boolean);
Procedure WriteFITS_RVB(p_prefix,p_no : String; p_bmp : TFastDIB; var pv_ok : boolean);

implementation

Const  MaxW=4096;


Function Slash(n : string) : string;
begin
if copy(n,length(n),1)<>'\' then result:=n+'\'
                            else result:=n;
end;


Procedure WriteFITS(p_nom,p_commentaire : String; p_typfit : TTypeFITS; p_bmp : TFastDIB; var pv_ok : boolean);
var l_header : FITSheader ;
    l_buf16  : FITSrec16;
    l_buf8   : FITSrec8;
    l_fitsfile : file ;
    n,i,j,k : integer;
    s : shortstring;
    l_st : TSYSTEMTIME;
    l_temp  : TDateTime;
    l_date,
    l_heure : String;
    l_imgw,
    l_imgh  : Integer;
    l_coul  : TFColor;

begin
GetSystemTime(l_st);
l_temp:=SystemTimeToDateTime(l_st);

l_date:=datetostr(l_temp);
l_heure:=timetostr(l_temp);

{nom:=nom+'.fit';}
if fileexists(p_nom) then begin
   i := messagedlg('Remplacer le fichier '+p_nom,mtWarning,[mbYes,mbNo],0);
   case i of
        mrYes : pv_ok:=true;
        mrNo  : begin
                pv_ok:=false;
                exit;
                end;
   end;
end;

l_imgw:=p_bmp.Width;
l_imgh:=p_bmp.Height;

i:=1;
FillChar(l_header, SizeOf(l_header), ' ');
s:='SIMPLE  =                    T';
move(s[1],l_header[i,1],length(s)); inc(i);

if g_paramfits_temp.bits_8 then
   begin
   s:='BITPIX  =                    8';
   move(s[1],l_header[i,1],length(s));inc(i);
   end
else
   begin
   s:='BITPIX  =                   16';
   move(s[1],l_header[i,1],length(s));inc(i);
   end;

s:='NAXIS   =                    2';
move(s[1],l_header[i,1],length(s));inc(i);
s:='NAXIS1  =                  '+inttostr(l_ImgW);
move(s[1],l_header[i,1],length(s));inc(i);
s:='NAXIS2  =                  '+inttostr(l_ImgH);
move(s[1],l_header[i,1],length(s));inc(i);
{if form1.radiogroup1.itemindex=0 then begin
  s:='DATAMAX =                   63'; move(s[1],l_header[i,1],length(s));inc(i);
  s:='DATAMIN =                    0'; move(s[1],l_header[i,1],length(s));inc(i);
  s:='THRESH  =                   63'; move(s[1],l_header[i,1],length(s));inc(i);
  s:='THRESL  =                    0'; move(s[1],l_header[i,1],length(s));inc(i);
end else begin
  s:='DATAMAX =                  255'; move(s[1],l_header[i,1],length(s));inc(i);
  s:='DATAMIN =                    0'; move(s[1],l_header[i,1],length(s));inc(i);
  s:='THRESH  =                  255'; move(s[1],l_header[i,1],length(s));inc(i);
  s:='THRESL  =                    0'; move(s[1],l_header[i,1],length(s));inc(i);
end;}

s:='DATAMAX =                  255';
move(s[1],l_header[i,1],length(s));inc(i);
s:='DATAMIN =                    0';
move(s[1],l_header[i,1],length(s));inc(i);
s:='THRESH  =                  255';
move(s[1],l_header[i,1],length(s));inc(i);
s:='THRESL  =                    0';
move(s[1],l_header[i,1],length(s));inc(i);

s:='DATE-OBS= '''+l_date+'T'+l_heure+'''' ; move(s[1],l_header[i,1],length(s));inc(i);
s:='OBJECT  = '''+g_paramfits_temp.Objet+'''' ; move(s[1],l_header[i,1],length(s));inc(i);
s:='TELESCOP= '''+g_paramfits_temp.Telescope+'''' ; move(s[1],l_header[i,1],length(s));inc(i);
s:='INSTRUME= '''+g_paramfits_temp.Instrument+'''' ; move(s[1],l_header[i,1],length(s));inc(i);
s:='OBSERVER= '''+g_paramfits_temp.Observateur+'''' ; move(s[1],l_header[i,1],length(s));inc(i);
s:='COMMENT  '+l_date+' '+l_heure+' UTC' ; move(s[1],l_header[i,1],length(s));inc(i);
s:='COMMENT   '+g_paramfits_temp.Commentaire ; move(s[1],l_header[i,1],length(s));inc(i);
if p_commentaire<>''
   then begin
        s:='COMMENT   '+p_commentaire ;
        move(s[1],l_header[i,1],length(s));
        inc(i);
        end;

s:='DATE    = '''+l_date+'''' ; move(s[1],l_header[i,1],length(s));inc(i);
s:='HISTORY  '+' FITS Created by AVI2BMP  '+l_date+' '+l_heure; move(s[1],l_header[i,1],length(s));inc(i);

{if form1.radiogroup1.itemindex=0 then begin s:='HISTORY  '+' Pixels values range from 0 (black) to 63 (white) '; move(s[1],l_header[i,1],length(s));inc(i);end
   else begin s:='HISTORY  '+' Pixels values are mean from original RGB '; move(s[1],l_header[i,1],length(s));inc(i);end;}

s:='HISTORY  '+' Pixels values are mean from original RGB '; move(s[1],l_header[i,1],length(s));inc(i);
s:='END      ' ; move(s[1],l_header[i,1],length(s));

assignfile(l_fitsfile,p_nom);
rewrite(l_fitsfile,1);
blockwrite(l_fitsfile,l_header,sizeof(l_header),n);
k:=0;
for j:=l_imgH-1 downto 0 do
  begin
  for i:=0 to l_ImgW-1 do
    begin
    inc(k);
    l_coul:=p_bmp.Pixels[j,i];
    if g_paramfits_temp.bits_8 then
       begin
       case p_typfit of
         fitRVB : l_buf8[k]:=(l_coul.r+l_coul.b+l_coul.g) div 3;
         fitR   : l_buf8[k]:=l_coul.r;
         fitV   : l_buf8[k]:=l_coul.g;
         fitB   : l_buf8[k]:=l_coul.b;
       end;
       if k=2880 then
          begin
          blockwrite(l_fitsfile,l_buf8,sizeof(l_buf8),n);
          k:=0;
          end;
       end
    else
       begin
       {l_buf16[k]:=(l_coul.r+l_coul.b+l_coul.g) div 3;}
       case p_typfit of
         fitRVB : l_buf16[k]:=(l_coul.r+l_coul.b+l_coul.g) div 3;
         fitR   : l_buf16[k]:=l_coul.r;
         fitV   : l_buf16[k]:=l_coul.g;
         fitB   : l_buf16[k]:=l_coul.b;
       end;
       l_buf16[k]:=swap(l_buf16[k]);
       if k=1440 then
          begin
          blockwrite(l_fitsfile,l_buf16,sizeof(l_buf16),n);
          k:=0;
          end;
       end;
    end;
  end;

if g_paramfits_temp.bits_8 then begin
  if k>0 then begin
     for i:=k+1 to 2880 do l_buf8[i]:=0;
     blockwrite(l_fitsfile,l_buf8,sizeof(l_buf8),n);
  end;
end else begin
  if k>0 then begin
     for i:=k+1 to 1440 do l_buf16[i]:=0;
     blockwrite(l_fitsfile,l_buf16,sizeof(l_buf16),n);
  end;
end;
closefile(l_fitsfile);
end;

Procedure WriteFITS_RVB(p_prefix,p_no : String; p_bmp : TFastDIB; var pv_ok : boolean);
begin
WriteFITS(p_prefix+'R'+p_no+'.fit','Composante Rouge',fitR,p_bmp,pv_ok);
WriteFITS(p_prefix+'V'+p_no+'.fit','Composante Verte',fitV,p_bmp,pv_ok);
WriteFITS(p_prefix+'B'+p_no+'.fit','Composante Bleu',fitB,p_bmp,pv_ok);
end;

{Procedure WriteFITScol(nom,seq,dat,tim : string; var ok : boolean);  // R G B
var header : FITSheader ;
    buf16r,buf16g,buf16b : FITSrec16;
    buf8r,buf8g,buf8b    : FITSrec8;
    fr,fg,fb : file ;
    n,i,j,k : integer;
    s : shortstring;
    buf : string;
    P : PbyteArray;
begin
buf:=nom+'_r'+seq+'.fit';
if fileexists(buf) and (not confirmreplace) then begin
   i := messagedlg('Remplacer le fichier '+buf,mtWarning,[mbYes,mbAll,mbNo],0);
   case i of
        mrAll : confirmreplace:=true;
        mrYes : ok:=true;
        mrNo  : begin
                ok:=false;
                exit;
                end;
   end;
end;
i:=1;
FillChar(header, SizeOf(header), ' ');
s:='SIMPLE  =                    T'; move(s[1],l_header[i,1],length(s));inc(i);
if form1.radiobutton1.checked then begin
   s:='BITPIX  =                    8'; move(s[1],l_header[i,1],length(s));inc(i); end
else begin
   s:='BITPIX  =                   16'; move(s[1],l_header[i,1],length(s));inc(i); end;
s:='NAXIS   =                    2'; move(s[1],l_header[i,1],length(s));inc(i);
s:='NAXIS1  =                  '+inttostr(ImgW); move(s[1],l_header[i,1],length(s));inc(i);
s:='NAXIS2  =                  '+inttostr(ImgH); move(s[1],l_header[i,1],length(s));inc(i);
s:='DATAMAX =                  255'; move(s[1],l_header[i,1],length(s));inc(i);
s:='DATAMIN =                    0'; move(s[1],l_header[i,1],length(s));inc(i);
s:='THRESH  =                  255'; move(s[1],l_header[i,1],length(s));inc(i);
s:='THRESL  =                    0'; move(s[1],l_header[i,1],length(s));inc(i);
s:='DATE-OBS= '''+dat+'T'+tim+'''' ; move(s[1],l_header[i,1],length(s));inc(i);
s:='OBJECT  = '''+form1.edit4.text+'''' ; move(s[1],l_header[i,1],length(s));inc(i);
s:='TELESCOP= '''+form1.edit5.text+'''' ; move(s[1],l_header[i,1],length(s));inc(i);
s:='INSTRUME= '''+form1.edit6.text+'''' ; move(s[1],l_header[i,1],length(s));inc(i);
s:='OBSERVER= '''+form1.edit7.text+'''' ; move(s[1],l_header[i,1],length(s));inc(i);
s:='COMMENT  '+dat+' '+tim+' UTC' ; move(s[1],l_header[i,1],length(s));inc(i);
s:='COMMENT   '+form1.edit3.text ; move(s[1],l_header[i,1],length(s));inc(i);
s:='DATE    = '''+datetostr(date)+'''' ; move(s[1],l_header[i,1],length(s));inc(i);
s:='HISTORY  '+' Original: '+currentfile ; move(s[1],l_header[i,1],length(s));inc(i);
s:='HISTORY  '+' FITS Convertion by QcamCopy  '+datetostr(date)+' '+timetostr(time) ; move(s[1],l_header[i,1],length(s));inc(i);
s:='HISTORY  '+' Pixels values are the RED layer from original RGB  '; j:=i;move(s[1],l_header[j,1],length(s));inc(i);
s:='END      ' ; move(s[1],l_header[i,1],length(s));
assignfile(fr,nom+'_r'+seq+'.fit');
rewrite(fr,1);
blockwrite(fr,l_header,sizeof(header),n);
s:='HISTORY  '+' Pixels values are the GREEN layer from original RGB    '; move(s[1],l_header[j,1],length(s));
assignfile(fg,nom+'_g'+seq+'.fit');
rewrite(fg,1);
blockwrite(fg,l_header,sizeof(header),n);
s:='HISTORY  '+' Pixels values are the BLUE layer from original RGB     '; move(s[1],l_header[j,1],length(s));
assignfile(fb,nom+'_b'+seq+'.fit');
rewrite(fb,1);
blockwrite(fb,l_header,sizeof(header),n);
k:=0;
for j:=0 to ImgH-1 do begin
  P:=imabmp.ScanLine[ImgH-1-j];
  for i:=0 to ImgW-1 do begin
    inc(k);
    if form1.RadioButton1.checked then begin        // 8 bit fits
        if imabmp.PixelFormat=pf24bit then begin // bmp 24 bit
         buf8r[k]:=P[3*i+2];
         buf8g[k]:=P[3*i+1];
         buf8b[k]:=P[3*i];
        end else begin                           // bmp 32 bit
         buf8r[k]:=P[4*i+2];
         buf8g[k]:=P[4*i+1];
         buf8b[k]:=P[4*i];
        end;
      if k=2880 then begin
         blockwrite(fr,buf8r,sizeof(buf8r),n);
         blockwrite(fg,buf8g,sizeof(buf8g),n);
         blockwrite(fb,buf8b,sizeof(buf8b),n);
         k:=0;
      end;
    end else begin                                  // 16 bit fits
      if imabmp.PixelFormat=pf24bit then begin // bmp 24 bit
      buf16r[k]:=swap(P[3*i+2]);
      buf16g[k]:=swap(P[3*i+1]);
      buf16b[k]:=swap(P[3*i]);
      end else begin                           // bmp 32 bit
      buf16r[k]:=swap(P[4*i+2]);
      buf16g[k]:=swap(P[4*i+1]);
      buf16b[k]:=swap(P[4*i]);
      end;
      if k=1440 then begin
         blockwrite(fr,buf16r,sizeof(buf16r),n);
         blockwrite(fg,buf16g,sizeof(buf16g),n);
         blockwrite(fb,buf16b,sizeof(buf16b),n);
         k:=0;
      end;
    end;
  end;
end;
if form1.RadioButton1.checked then begin
  if k>0 then begin
     for i:=k+1 to 2880 do buf8r[i]:=0;
     for i:=k+1 to 2880 do buf8g[i]:=0;
     for i:=k+1 to 2880 do buf8b[i]:=0;
     blockwrite(fr,buf8r,sizeof(buf8r),n);
     blockwrite(fg,buf8g,sizeof(buf8g),n);
     blockwrite(fb,buf8b,sizeof(buf8b),n);
  end;
end else begin
  if k>0 then begin
     for i:=k+1 to 1440 do buf16r[i]:=0;
     for i:=k+1 to 1440 do buf16g[i]:=0;
     for i:=k+1 to 1440 do buf16b[i]:=0;
     blockwrite(fr,buf16r,sizeof(buf16r),n);
     blockwrite(fg,buf16g,sizeof(buf16g),n);
     blockwrite(fb,buf16b,sizeof(buf16b),n);
  end;
end;
closefile(fr);
closefile(fg);
closefile(fb);
end;}

{confirmreplace:=false; ok :=true;
DateSeparator:='-';
ShortDateFormat:='yyyy-mm-dd';
TimeSeparator:=':';
LongTimeFormat:='hh:mm:ss';
{$I-}
{decimalseparator:='.';}
{$I+}
{GetLocalTime(lt);
GetSystemTime(st);
bias:=SystemTimeToDateTime(st)-SystemTimeToDateTime(lt);
Datetimepicker1.datetime:=now+bias;
Datetimepicker2.datetime:=now+bias;
}

end.
*/
