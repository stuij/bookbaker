typedef unsigned short size_t; 

#include "gba.h" 
#include "screenmode.h"
#include "keypad.h"
#include "gbfs.h"
#include "book.h"

volatile u16*scanlineCounter = (volatile u16*) 0x4000006;
u8 *cartSave = (u8*)0x0E000000;

u16 *vidPage = FrontBuffer;
u16 vidFlip = 0xA000;

u16 posPointer = 0;

// start da program!!!

int main(void)
{
  struct offsTimeline textPos;
  u32 size = 0;
  enum direction scrollDir = forward;

  SetMode(MODE_4 | BG2_ENABLE);
  
  BGPaletteMem[0] = 0;
  BGPaletteMem[1] = RGB(31,31,31);
 
  /* Start searching from the beginning of cartridge memory */
  const GBFS_FILE * gbfs_file = find_first_gbfs_file((void*)0x08000000);
  char* text = (char*)gbfs_get_obj(gbfs_file, "text.txt", &size);
  
  if(size == 0)
    {
      text = "no file found";
      size = sizeof text / sizeof *text;
    }

  if(cartSave[0] != 177)
    {
      cartSave[0] = 177;    
      WritePos(0);
      WritePos2(0);
      textPos.offs=0;
    }
  else
    textPos.offs=ReadPos();
  
  BlackOut();
  textPos = FillFromScratch(textPos, text, size);
  
  while(1)
    {
      if (!((*KEYS)& KEY_A) || !((*KEYS)& KEY_L) || !((*KEYS)& KEY_R))
        {
          if(scrollDir==back)
            {
              BlackOut();
              textPos.offs = TextFw(text,size,textPos.offsOldest);
              textPos = dirChangeDance(textPos);
              scrollDir=forward;
            }
          textPos = PrepareForPageJump(textPos);
          textPos.offs = TextFw(text,size,textPos.offs);
        }

      if (! ((*KEYS)& KEY_B))
        {
          if(scrollDir==forward)
            {
              BlackOut();
              textPos.offs = TextBw(text,size,textPos.offsOldest);
              textPos = dirChangeDance(textPos);
              scrollDir=back;
            }

          textPos = PrepareForPageJump(textPos);
          textPos.offs = TextBw(text,size,textPos.offs);
        }
      
      if (! ((*KEYS)& KEY_START))
        {
          WritePos(textPos.offsOldest);
          REG_DISPCNT ^= BACKBUFFER;
          BlackOut();
          TextFw(text,size,textPos.offsOldest);
          REG_DISPCNT ^= BACKBUFFER;
        }

      if (! ((*KEYS)& KEY_RIGHT))
        {
          textPos = SearchFw(textPos, text, size, &scrollDir);
        }

      if (! ((*KEYS)& KEY_LEFT))
        {
          textPos = SearchBw(textPos, text, size, &scrollDir);
        }
            
    }
}


// general c functions
void SwapArr(int* a, u32 size)
{
  u32 i;
  u32 j;
  u32 mid = size >> 1;
  
  for (i = 0, j = size-1-i; i < mid ; i++, j = size-1-i)
    a[i] ^= a[j] ^= a[i] ^= a[j]; // stupid xor trick - it exchanges the values of i and j
}

// gba functions
void WritePos(u32 offset)
{
  cartSave[1] = offset;
  cartSave[2] = offset >> 8;
  cartSave[3] = offset >> 16;
  cartSave[4] = offset >> 24;
}

u32 ReadPos()
{
  u32 a0,a1,a2,a3;
  
  a0 = cartSave[1];
  a1 = cartSave[2];
  a2 = cartSave[3];
  a3 = cartSave[4];

  return a0 | (a1 << 8) | (a2 << 16) | (a3 << 24);
}

void WritePos2(u32 offset)
{
  cartSave[5] = offset;
  cartSave[6] = offset >> 8;
  cartSave[7] = offset >> 16;
  cartSave[8] = offset >> 24;
}

u32 ReadPos2()
{
  u32 a0,a1,a2,a3;
  
  a0 = cartSave[5];
  a1 = cartSave[6];
  a2 = cartSave[7];
  a3 = cartSave[8];

  return a0 | (a1 << 8) | (a2 << 16) | (a3 << 24);
}

// write functions
void BlackOut()
{
  u32 x;
  u32 y;
  
  for (x = 0; x < 120; x++)
    for (y = 0; y < 160; y++)
      vidPage[x + y * 120] = 0;
}

void PlotChar(u32 ch, u32 offsetX, u32 offsetY, u8 color)
{
  u32 lchar=letterSize[ch];
  u32 l;
  u32 temp;
  u32 x;
  u32 y;
  
  for(l=0; l < lchar; l++)
    {
      x = (fontxData[fontOffset[ch] + l] + offsetX);
      y = (fontyData[fontOffset[ch] + l] + offsetY);
  
      temp = vidPage[(x + y * 240) >> 1];
      
      if(x & 1)
        temp = (temp & 0xFF) + (color << 8);       
      else
        temp = (temp & 0xFF00) + color;
      
      vidPage[(x + y * 240) >> 1] = temp;
    } 
}
  
void PlotCharString(int* ch, u32 size, u32 yOffs)
{
  u32 offs = 0;
  u32 offsnew = 0;
  u32 l;
  
  for (l=0; l< size; l++)
    {
      offs = offsnew;
      offsnew += letterOffset[ch[l]] + 2;

      PlotChar(ch[l],offs+1,yOffs,1);
    }
}


// ebook functions
int TextFw(char* text, u32 size, u32 offs)
{ 
  int offSpace=0; // space offset in letterstream, eg, indicates when the last space was seen so we can put the word that didn't fit on a
                  // line, on the next
  int ch[240]; // meer chars dan dit zullen er niet op lijn komen
  int lineOffset=2;  // want in plotcharstring ook om van rand weg te komen 
  int yOffs = 4; // vier pixels van bovenrand beginnen
  int yCounter;
  int l;
  int i;
  
  for (yCounter=0; yCounter < 11; yCounter++) //nr of lines on screen
    {
      for (l=0 ; l< 240; l++) //loop till max size of ch array
        {
          for (i=0; i<256; i++) //check character code of character
            if (text[l + offs] == (char)i)	  
              {
                lineOffset += letterOffset[i] + 2; //collect offset info

                switch (i)
                  {
                  case 32:
                    offSpace=l;
                    break;
                  case 13:
                    offSpace=l;
                    lineOffset = 241;
                    break;
                  case 10:
                    if(!(text[l+offs-1] == '\r'))
                      {
                        offSpace=l;
                        lineOffset = 241;
                      }
                    break;                 
                  default:
                    break;
                  }
                  
                ch[l] = i; //record character
              }
          
          if (lineOffset > 240) //if offset is bigger than width of screen, go to next line
            break; 
        }
      PlotCharString(ch, offSpace, yOffs);
      yOffs += 14;
      lineOffset = 2;
      offs+=offSpace +1;
      offSpace=0;
    }    
  return (offs);
}

int TextBw(char* text, u32 size, u32 offs) // similar to previous function but than mirrored to go back,
{                                            // plus swapping of lines and line arr
  int offSpace=0; // space offset in letterstream, eg, indicates when the last space was seen so we can put the word that didn't fit on a
                  // line, on the next
  int ch[240]; // meer chars dan dit zullen er niet op lijn komen
  int lineOffset=2;  // want in plotcharstring ook om van rand weg te komen 
  int yOffs = 144; // vier pixels van bovenrand beginnen
  int yCounter;
  int l;
  int i;

  offs = offs - 2;

  for (yCounter=0; yCounter < 11; yCounter++) //nr of lines on screen
    {
      for (l=0 ; l<240; l++) //loop till max size of ch array
        {
          for (i=0; i<256; i++) //check character code of character
            if (text[offs - l] == (char)i)	  
              {
                lineOffset += letterOffset[i] + 2; //collect offset info

                switch (i)
                  {
                  case 32:
                    offSpace=l;
                    break;
                  case 13:
                    offSpace=l;
                    lineOffset = 241;
                    break;
                  case 10:
                    if(!(text[offs- (l + 1)] == '\r'))
                      {
                        offSpace=l;
                        lineOffset = 241;
                      }
                    break;
                  default:
                    break;
                  }
                      
                ch[l] = i; //record character
              }
          
          if (lineOffset > 240) //if offset is bigger than width of screen, go to next line
            break; 
        }
      SwapArr(ch,offSpace);
      PlotCharString(ch, offSpace, yOffs);
      yOffs -= 14;
      lineOffset = 2;
      offs -= offSpace + 1;
      offSpace=0;
    }    
  return (offs + 2);
}

struct offsTimeline FillFromScratch(struct offsTimeline textPos, char* text, u32 size)
{
  vidPage = (u16*)((u32)vidPage ^ vidFlip);
  BlackOut();
  textPos.offsOldest = textPos.offs;
  textPos.offs = TextFw(text,size,textPos.offs);
  REG_DISPCNT ^= BACKBUFFER;
  vidPage = (u16*)((u32)vidPage ^ vidFlip);
  BlackOut();
  textPos.offsOlder = textPos.offs;
  textPos.offs = TextFw(text,size,textPos.offs);

  return textPos;
}

struct offsTimeline PrepareForPageJump(struct offsTimeline textPos)
{
  REG_DISPCNT ^= BACKBUFFER;
  vidPage = (u16*)((u32)vidPage ^ vidFlip);
  BlackOut();
  textPos.offsOldest = textPos.offsOlder;
  textPos.offsOlder = textPos.offs;

  return textPos;
}

struct offsTimeline dirChangeDance(struct offsTimeline textPos)
{
  textPos.offsOldest ^= textPos.offsOlder ^= textPos.offsOldest ^= textPos.offsOlder; // stupid xor trick 
  return textPos;
}

struct offsTimeline SearchFw(struct offsTimeline textPos, char* text, u32 size, enum direction* scrollDir)
{
  u32 offs;
  
  for(offs = textPos.offsOldest +1; offs<size; offs++)
    {
      if(text[offs] == '~' && (text[offs - 1] == '\r' || text[offs - 1] == '\n'))
        {
          textPos.offs = offs;
          vidPage = (u16*)((u32)vidPage ^ vidFlip);
          textPos = FillFromScratch(textPos, text, size);
          *scrollDir = forward;
          break;
        }
    }
  return textPos;
}

struct offsTimeline SearchBw(struct offsTimeline textPos, char* text, u32 size, enum direction* scrollDir)
{
  u32 offs;
  
  for(offs = textPos.offsOldest - 1; offs>0; offs--)
    {
      if(text[offs] == '~' && (text[offs - 1] == '\r' || text[offs - 1] == '\n'))
        {
          textPos.offs = offs;
          vidPage = (u16*)((u32)vidPage ^ vidFlip);
          textPos = FillFromScratch(textPos, text, size);
          *scrollDir = forward;
          break;
        }
    }
  return textPos;
}
