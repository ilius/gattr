#include <gtkmm.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string.h>
#include <stdio.h> // for popen

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


using namespace std;

class AttrItem{
public:
  int num;
  Glib::ustring sym; // or char ?????
  Glib::ustring desc;
  AttrItem(const int num, const Glib::ustring sym, const Glib::ustring desc){
    this->num = num;
    this->sym = sym;
    this->desc = desc;
  }
};

#define ATTR_NUM 12
static const AttrItem attributes[ATTR_NUM]={
AttrItem(0 , "s", "Secure delete"),
AttrItem(1 , "u", "Undeletable"),
AttrItem(2 , "S", "Synchronous update"),
AttrItem(3 , "D", "Synchronous directory update"),
AttrItem(4 , "i", "Immutable"),
AttrItem(5 , "a", "Append only"),
AttrItem(6 , "d", "No dump"),
AttrItem(7 , "A", "Don't update access time"),
AttrItem(8 , "c", "Compress"),
// 9, 10, 11, 12 ??????????
AttrItem(13, "j", "Data journal"),
// 14 ??????
AttrItem(15, "t", "?"),
AttrItem(16, "T", "?")
// 17, 18
};

// "D" is not valid for file (only for directory)
// "i" change needs root access
// "a" change needs root access
// "j" change needs root access




int str2int(string s){
  int n;
  stringstream ss;
  ss << s;
  ss >> n;
  return n;
}
string tostr(int n){
  ostringstream oss;
  oss << n;
  return oss.str();
}

char *str2charp(Glib::ustring s){
  //return s.c_str();
  int n = s.size();
  char *ch = new char[n+1];
  s.copy(ch, n);
  ch[n] = 0;
  return ch;
}
/*
char *char2str(char c){
  char *s = new char[2];
  s[0] = c;
  s[1] = 0;
  return s;
}

char *read_file(FILE *fp){
  if ((fseek(fp, 0, SEEK_END)) != 0) {
    cout << "Error in seek operation on file!\n";
    return "";
  }
  // Set filesize variable    
  int filesize = ftell(fp);
  // Go back
  rewind(fp); 
  ///////////
  if(ferror(fp)){
    cout << "Error reading file!\n";
    return "";
  }
  char *text = new char[filesize+1];
  fread(text, 1, filesize, fp);
  return text;
}*/

class AttrBox: public Gtk::VBox{
public:
  bool opened;
  char *filename;
  bool isdir;
  ////////////
  Gtk::Label label;
  Gtk::TreeView treev;
  Glib::RefPtr<Gtk::ListStore> trees;
  Gtk::TreeModel::Row row;
  vector<Gtk::TreeModel::Row> rows;
  Gtk::CellRendererText *cell;
  Gtk::CellRendererToggle *cellg;
  Gtk::TreeView::Column *col;
  Gtk::TreeModel::ColumnRecord col_rec;
  //////////
  Gtk::TreeModelColumn<bool> col_check;
  Gtk::TreeModelColumn<int> col_index;
  Gtk::TreeModelColumn<Glib::ustring> col_desc, col_sym;
  ///Gtk::TreeModelColumn<char> col_sym;

AttrBox():
Glib::ObjectBase(typeid(AttrBox)),
Gtk::VBox(),
label("")
{
  opened=false;
  isdir=false;
  int i, j;
  ////////
  col_rec.add(col_index);
  treev.append_column("Index", col_index);
  treev.get_column(0)->set_property("resizable", true);
  ////////
  col_rec.add(col_sym);
  treev.append_column("Symbol", col_sym);
  treev.get_column(1)->set_property("resizable", true);
  ////////
  col_rec.add(col_desc);
  treev.append_column("Description", col_desc);
  treev.get_column(2)->set_property("resizable", true);
  treev.get_column(2)->set_expand(true);
  ////////
  col_rec.add(col_check);
  /*treev.append_column_editable("Enable", col_check);
  col = treev.get_column(3);
  col->set_property("resizable", true);
  col->set_cell_data_func(*treev.get_column_cell_renderer(3),
    sigc::mem_fun(*this,&AttrBox::toggle_data_func));*/
  cellg = new Gtk::CellRendererToggle();
  col = new Gtk::TreeView::Column();
  col->set_title("Enable");
  col->add_attribute(*cellg, "active", 3);
  col->pack_start(*cellg, false); // second arg: expand=true
  //col->set_expand(false);
  treev.append_column(*col);
  col->set_cell_data_func(*cellg,sigc::mem_fun(*this,&AttrBox::toggle_data_func));
  ///cell->set_property("editable", true); // Make Seg fault
  cellg->signal_toggled().connect( sigc::mem_fun(*this, &AttrBox::toggle_cell_edited) );
  /////////////////////
  trees = Gtk::ListStore::create(col_rec);
  treev.set_model(trees);  
  /////////////////////
  if(getuid()!=0){
    label.set_use_markup(true);
    label.set_selectable(true);
    label.set_alignment(0, 0.5);
    this->pack_start(label, 0, 0);
  }
  this->pack_start(treev);
  //label.set_label("No file opened");
  treev.set_sensitive(false);
} // AttrBox()
void toggle_cell_edited(const Glib::ustring& path_str)
{
  //int i = str2int(path_str);
  //if( isdir || rows[i][col_sym]!="D" )
  //  rows[i][col_check] = !rows[i][col_check];
  //////////////////////////////
  Gtk::TreePath path(path_str);
  Gtk::TreeModel::Row row = (*(trees->get_iter(path)));
  if( isdir || row[col_sym]!="D" ){
    bool active = !row[col_check];
    if(set_file_attr(row[col_sym], active))
      row[col_check] = active;
  }
}
void toggle_data_func(Gtk::CellRenderer* cl, const Gtk::TreeModel::iterator& iter){
    if(iter)
      cl->set_property("active", (gboolean)((*iter)[col_check]));
}
bool load_file(char *filename){
  cout << "Loading file \"" << filename << "\"\n";
  int i, j, rowNum;
  struct stat fst;
  this->filename = filename; 
  int ist = stat(filename, &fst);
  cout << "stat output: " << ist << "\n";
  if(ist!=0){
    cout<<"error: could not read stat of file '"<<filename<<
      "', maybe no such file/directory! error number: "<<ist<<"\n";
    return false;
  }
  cout << "st_mode=" << fst.st_mode << "\n";
  isdir = S_ISDIR(fst.st_mode);
  cout << "isdir=" << isdir << "\n";
  /////////////////////////////
  trees->clear();
  for(i=0; i<ATTR_NUM; i++){
    if(attributes[i].sym=="D" && !isdir)
      continue;
    row = *(trees->append());
    rows.push_back(row);
    row[col_index] = attributes[i].num;
    row[col_sym] = attributes[i].sym;
    row[col_desc] = attributes[i].desc;
    row[col_check] = false;
  }
  //////////////////////////////
  if(getuid()!=0){
    if(isdir)
      label.set_label(
      "Changing attributes <b>D</b>, <b>i</b>, <b>a</b> and <b>j</b> needs root privilege.");
    else
      label.set_label(
      "Changing attributes <b>i</b>, <b>a</b> and <b>j</b> needs root privilege.");
    //label.show();
  }
  //else
  //  label.hide();
  /////////////////////////////
  char cmd[strlen(filename)+15];
  if(isdir)
    sprintf(cmd, "lsattr -d '%s'", filename);
  else
    sprintf(cmd, "lsattr '%s'", filename);
  char out_ch[1000];
  FILE *p = popen(cmd, "r");
  //cout << read_file(p) << "\n";
  fscanf (p, "%s", out_ch);
  Glib::ustring out=out_ch;
  pclose(p);
  ///////////////
  cout << "lsattr output: " << out << "\n";
  int n=out.size();
  if(n!=19){
    cout << "error: bad output: size!=19\n";
    return false;
  }
  Glib::ustring sub;
  for(i=0; i<n; i++){
    sub = out.substr(i, 1);
    if(sub=="-")
      continue;
    //if(sub!=attributes[i].sym){
    //  cout "unexpected attribute symbol '"<<sub<<"' at index"<<i<<"\n";
    //  continue;
    //}
    rowNum = -1;
    for(j=0; j<rows.size(); j++){
      if(rows[j][col_sym]==sub){
        rowNum = j;
        break;
      }
    }
    if(rowNum==-1){
      cout << "error: unknown symbol '" << sub << "'\n";
      continue;
    }
    if(rows[rowNum][col_index]!=i){
      cout << "warning: unexpected index "<<i<<" for symbol '"<<sub<<"'\n";
      //continue;
    }
    rows[rowNum][col_check] = true;
  }
  treev.set_sensitive(true);
  opened = true;
  return true;
}
bool load_file(Glib::ustring filename){
  load_file(str2charp(filename));
}
bool set_file_attr(Glib::ustring sym, bool active){
  Glib::ustring str;
  char cmd[15+strlen(filename)]; // chattr +u '/media/...'
  if(active)
    sprintf(cmd, "chattr +%s '%s'", sym.c_str(), filename);
  else
    sprintf(cmd, "chattr -%s '%s'", sym.c_str(), filename);
  int ex = system(cmd);
  if(ex==0)
    return true;
  else{
    cout<<"error while changing attribute '"<<sym<<"', errno: "<<ex<<"\n\n";
    return false;
  }
}
};// class AttrBox

