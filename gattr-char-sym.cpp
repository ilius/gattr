//rs; g++ `pkg-config --cflags --libs gtkmm-2.4` gattr.cpp -o gattr
#include <gtkmm.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

class AttrItem{
public:
  int num;
  char sym;
  Glib::ustring desc;
  AttrItem(int num, const char sym, Glib::ustring desc){
    this->num = num;
    this->sym = sym;
    this->desc = desc;
  }
};

#define ATTR_NUM 12
AttrItem attributes[]={
AttrItem(0 , 's', "Secure delete"),
AttrItem(1 , 'u', "Undeletable"),
AttrItem(2 , 'S', "Synchronous update"),
AttrItem(3 , 'D', "Synchronous directory update"),
AttrItem(4 , 'i', "Immutable"),
AttrItem(5 , 'a', "Append only"),
AttrItem(6 , 'd', "No dump"),
AttrItem(7 , 'A', "Dont update access time"),
AttrItem(8 , 'c', "Compress"),
// 9, 10, 11, 12 ??????????
AttrItem(13, 'j', "Data journal"),
// 14 ??????
AttrItem(15, 't', ""),
AttrItem(16, 'T', "")
// 17, 18
};

// "D" is not valid for file (only for directory)
// "i" needs root access
// "a" needs root access
// "j" needs root access




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
char *char2str(char c){
  char *s = new char[2];
  s[0] = c;
  s[1] = 0;
  return s;
}


class AttrBox: public Gtk::VBox{
public:
  Glib::ustring filename;
  bool isdir;
  ////////////
  Gtk::Label *label;
  Gtk::TreeView treev;
  Glib::RefPtr<Gtk::ListStore> trees;
  vector<Gtk::TreeModel::Row> rows;
  Gtk::CellRendererText *cell;
  Gtk::CellRendererToggle *cellg;
  Gtk::TreeView::Column *col;
  Gtk::TreeModel::ColumnRecord col_rec;
  //////////
  Gtk::TreeModelColumn<bool> col_check;
  Gtk::TreeModelColumn<int> col_index;
  Gtk::TreeModelColumn<Glib::ustring> col_desc;
  //Gtk::TreeModelColumn<Glib::ustring> col_sym;
  Gtk::TreeModelColumn<char> col_sym;

AttrBox(Glib::ustring filename):
Glib::ObjectBase(typeid(AttrBox)),
Gtk::VBox()
{
  this->filename;
  isdir = false;//??????????????
  ////////
  int i;
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
  col->pack_start(*cellg);
  treev.append_column(*col);
  col->set_cell_data_func(*cellg,sigc::mem_fun(*this,&AttrBox::toggle_data_func));
  cellg->signal_toggled().connect( sigc::mem_fun(*this, &AttrBox::toggle_cell_edited) );
  /////////////////////
  trees = Gtk::ListStore::create(col_rec);
  treev.set_model(trees);
  for(i=0; i<ATTR_NUM; i++)
    rows.push_back(*(trees->append()));
  /////////////////////
  for(i=0; i<ATTR_NUM; i++){
    if(attributes[i].sym=='D' && !isdir)
      continue;
    rows[i][col_index] = attributes[i].num;
    //rows[i][col_sym] = char2str(attributes[i].sym);
    rows[i][col_sym] = attributes[i].sym;
    rows[i][col_desc] = attributes[i].desc;
    rows[i][col_check] = false;
  }
  //////////////////////////////
  this->pack_start(treev);
  load_file_attr();
} // AttrBox()
void toggle_cell_edited(const Glib::ustring& path_str)
{
  //int i = str2int(path_str);
  //if( isdir || rows[i][col_sym]!='D' )
  //  rows[i][col_check] = !rows[i][col_check];
  //////////////////////////////
  Gtk::TreePath path(path_str);
  Gtk::TreeModel::Row row = (*(trees->get_iter(path)));
  if( isdir || row[col_sym]!='D' )//?????????
    row[col_check] = !row[col_check];
}
void toggle_data_func(Gtk::CellRenderer* cl, const Gtk::TreeModel::iterator& iter){
    if(iter)
      cl->set_property("active", (gboolean)((*iter)[col_check]));
}
bool load_file_attr(){
  //system("lsattr '"+filename+"'"); // Howto get output ????????????????

}
bool set_file_attr(){
  Glib::ustring str;
  

}







};// class AttrBox

int main(int argc, char **argv){
  Gtk::Main kit(argc, argv);
  Gtk::Dialog win("File Attributes");
  AttrBox attr("test");
  win.get_vbox()->pack_start(attr, 0, 0);
  win.show_all();
  
  
  Gtk::Main::run(win);
  return 0;
}

