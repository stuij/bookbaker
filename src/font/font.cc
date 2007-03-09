// progje om fontfiletje omzetten in een stel int arrays

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

int main() {

  ifstream in("font");
  ofstream out("font.c");
  string x,y,z,line;
  int i=0;
  int j=0;
  int k=1;
  int kk=0;
  stringstream fontoffset;
  stringstream lettersize;
  stringstream letteroffset;
  
  //bestand naar aparte strings
  for (i=0; i<34; i++)
    fontoffset << "0,";

  for (i=0; i<33; i++)
    lettersize << "0,";
    

  for (i=0; i<32; i++)
    letteroffset << "-1,";

  letteroffset << "2,";

  i=0;

  while(getline(in, line))
    {
      if (i==1)
	{
	  x += line + ",";


	  // offset array
	  for (j=0; j < line.size(); j++)
	    {
	      if(line[j]==char(44))
		k++;
	    }  
	  
	  kk += k;
	  
	  if (line[line.size() - 2] == char(44))
	    letteroffset << line[line.size() - 1] << ",";
	  else
	    letteroffset << line[line.size() - 2] << line[line.size() - 1] << ",";
				 	 
	  lettersize << k  << ",";
	   
	  fontoffset << kk << ",";
	  k = 1;
	}
	  
      if (i==2)
	y += line + ",";
      
      i++;
      
      if (i > 3)
	i=0;
    }
  
  // Spaties uit string x + komma weg uit eind
  string x2;
  
  for (i=0; i<x.size(); i++)
    
    {
      if (x[i] != char(32))
        x2+= x[i];
    }
  
  x2.erase(x2.size()-1);
  // spaties uit string y + komma weg uit eind
  string y2;
  
  for (i=0; i<y.size(); i++)
    
    {
      if (y[i] != char(32))
        y2+= y[i];
    }
       
  y2.erase(y2.size()-1);
  
  // stringstream naar string overzetten 
  string offset = fontoffset.str();

  // loze offsets toevoegen om aantal gelijk te stellen aan ascii
  string offset2;
  k=0;
  
  for (i=0; i<offset.size(); i++)
    {
      offset2+=offset[i];
      
      if (offset[i] == char(44))
	k++;

      if (k == 127)
	for (int w=127; w<161; w++)
	  {
	    k++;
	    offset2+= "0,";
	  }
    }      

  //komma er af schaven
  offset2.erase(offset2.size()-1);
  
  string loffset = letteroffset.str();
  string loffset2;
  
  for (i=0; i<loffset.size(); i++)
    {
      if (loffset[i] != char(32))
	loffset2+=loffset[i];
    }

  loffset2.erase(loffset2.size()-1);
  
  string loffset3;
  k=0;

  for (i=0; i<loffset2.size(); i++)
    {
      loffset3+=loffset2[i];

      if (loffset2[i] == char(44))
	k++;

      if (k == 127)
	for (int w=127; w<161; w++)
	  {
	    k++;
	    loffset3+= "-1,";
	  }
    }
  
  string lsize = lettersize.str();
  lsize.erase(lsize.size()-1);

  string lsize2;
  k=0;

  for (i=0; i<lsize.size(); i++)
    {
      lsize2+=lsize[i];

      if (lsize[i] == char(44))
	k++;

      if (k == 127)
	for (int w=127; w<161; w++)
	  {
	    k++;
	    lsize2+= "0,";
	  }
    }

  out << "/* font file, in elkaar geklust door cybertiesje */" << endl << endl << endl;
  out << "const int fontxData[] =" << endl << "{" << x2 << "};" << endl << endl;
  out << "const int fontyData[] =" << endl << "{" << y2 << "};" << endl << endl;
  out << "const int fontOffset[] =" << endl << "{" << offset2 << "};" << endl << endl;
  out << "const int letterOffset[] =" << endl << "{" << loffset3 << "};" << endl << endl;
  out << "const int letterSize[] =" << endl << "{" << lsize2 << "};" << endl << endl;
  out << endl;
  cout << endl << "font.c ligt versgebakken in deze folder" << endl << "pas op, hij is nog heet!!!" << endl << endl;
}
