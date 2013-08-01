//rs; g++ `pkg-config --cflags --libs gtkmm-2.4` gattr.cpp -o gattr

#include "attrbox.cpp"

class AttrDialog: public Gtk::Dialog{
public:
  Gtk::VBox *vbox;
  AttrBox abox;
  Gtk::Label label;
  Gtk::HBox hbox;
  Gtk::Button button;
  Gtk::Button *button_cancel, *button_fcd_open;
  Gtk::FileChooserDialog fcd;

AttrDialog():
Glib::ObjectBase(typeid(AttrDialog)),
Gtk::Dialog("gattr"),
label("No file opened"),
button(Gtk::Stock::OPEN),
fcd("Select File to Open", Gtk::FILE_CHOOSER_ACTION_OPEN)
{
  vbox = this->get_vbox();
  hbox.pack_start(label, 1, 1);
  hbox.pack_start(button, 0, 0);
  vbox->pack_start(hbox);
  vbox->pack_start(abox);
  label.set_alignment(0, 0.5);
  label.set_selectable(true);
  button.signal_clicked()
    .connect(sigc::mem_fun(*this, &AttrDialog::open_clicked), false);
  button_cancel = fcd.add_button(Gtk::Stock::CANCEL, 0);
  button_cancel->signal_clicked()
    .connect(sigc::mem_fun(*this, &AttrDialog::cancel_clicked), false);
  button_fcd_open = fcd.add_button(Gtk::Stock::OPEN, 0);
  button_fcd_open->signal_clicked()
    .connect(sigc::mem_fun(*this, &AttrDialog::fcd_open_clicked), false);
  fcd.signal_delete_event()
    .connect(sigc::mem_fun(*this, &AttrDialog::fcd_delete_event), false);
  fcd.signal_file_activated()
    .connect(sigc::mem_fun(*this, &AttrDialog::fcd_open_clicked), false);
  vbox->show_all();
}
void open_clicked(){
  fcd.run();
}
void fcd_open_clicked(){
  Glib::ustring fname=fcd.get_filename();
  abox.load_file(fname);
  label.set_label(fname);
  fcd.hide();
}
void open_file(char *filename){
  abox.load_file(filename);
  label.set_label(filename);
}
void cancel_clicked(){
  fcd.hide();
}
bool fcd_delete_event(GdkEventAny* event){
  fcd.hide();
  return true;
}
};// class AttrDialog

int main(int argc, char **argv){
  Gtk::Main kit(argc, argv);
  AttrDialog dialog;
  // parse command line arguments?????????????
  if(argc>1)
    dialog.open_file(argv[1]);
  Gtk::Main::run(dialog);
  return 0;
}

