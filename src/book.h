enum direction {back=0, forward};

struct offsTimeline
{
  u32 offs;
  u32 offsOlder;
  u32 offsOldest;
};
  
#define RGB(r,g,b) ((r)+((g)<<5)+((b)<<10))
#define BGR(b,g,r) ((r)+((g)<<5)+((b)<<10))

// function declarations
// general c functions
void swap(int* v, u32 i, u32 j);
void SwapArr(int* a, u32 size);

// gba functions
void WritePos(u32 offset);
u32 ReadPos();
void WritePos2(u32 offset);
u32 ReadPos2();

// general writing functions
void BlackOut();
void PlotChar(u32 ch, u32 offsetX, u32 offsetY, u8 color);
void PlotCharString(int* ch, u32 size, u32 yOffs);

// ebook functions
int TextFw(char* text, u32 size, u32 offs);
int TextBw(char* text, u32 size, u32 offs);
struct offsTimeline FillFromScratch(struct offsTimeline textPos, char* text, u32 size);
struct offsTimeline PrepareForPageJump(struct offsTimeline textPos);
struct offsTimeline dirChangeDance(struct offsTimeline textPos);
struct offsTimeline SearchFw(struct offsTimeline textPos, char* text, u32 size, enum direction* scrollDir);
struct offsTimeline SearchBw(struct offsTimeline textPos, char* text, u32 size, enum direction* scrollDir);

extern const int fontxData[];
extern const int fontyData[];
extern const int fontOffset[];
extern const int letterOffset[];
extern const int fontOffset[];
extern const int letterSize[];
