#!/usr/bin/env python

from __future__ import print_function

import pygtk
import gtk
pygtk.require('2.0')


import os
import shutil
import datetime
from fnmatch import fnmatch
import subprocess




class ConfChooser:

    # General Functions

    def update_combo(self,combo,list):
        combo.set_sensitive(False)
        combo.get_model().clear()
        for i in list:
            combo.append_text(i)
        combo.set_active(0)
        combo.set_sensitive(True)

    def update_label(self):
        desc = "Current conf: "
        if not os.path.lexists(self.conf_xml):
                desc += "does not exist"
        else:
            if os.path.islink(self.conf_xml):
                if os.path.exists(self.conf_xml):
                    desc += "symlink to "
                else:
                    desc += "broken symlink to "
            real_conf_path = os.path.realpath(self.conf_xml)
            desc += os.path.relpath(real_conf_path, self.conf_dir)
        self.explain.set_text(desc)

    # CallBack Functions


    def find_conf_files(self):

        conf_files = []

        pattern = "*conf.xml*"
        backup_pattern = "conf.xml.2[0-9][0-9][0-9]-[01][0-9]-[0-3][0-9]_*"
        excludes = ["conf.xml", "%gconf.xml"]

        for path, subdirs, files in os.walk(self.conf_dir):
            for name in files:
                if self.exclude_backups and fnmatch(name, backup_pattern):
                    continue
                if fnmatch(name, pattern):
                    filepath = os.path.join(path, name)
                    entry = os.path.relpath(filepath, self.conf_dir)
                    if not os.path.islink(filepath) and entry not in excludes:
                        conf_files.append(entry)

        conf_files.sort()
        self.update_combo(self.conf_file_combo, conf_files)


    def about(self):
        gui_dialogs.about(paparazzi.home_dir)

    def launch(self, widget):
        os.system("./paparazzi &");
        gtk.main_quit()

    def backupconf(self, use_personal=False):
        timestr = datetime.datetime.now().strftime("%Y-%m-%d_%H:%M")
        if os.path.islink(self.conf_xml):
            print("Symlink does not need backup");
        else:
            newname = "conf.xml." + timestr
            backup_file = os.path.join(self.conf_dir, newname)
            shutil.copyfile(self.conf_xml, backup_file)
            print("Made a backup: " + newname)

        if use_personal:
            conf_personal = os.path.join(self.conf_dir, "conf.xml.personal")
            conf_personal_backup = os.path.join(self.conf_dir, "conf.xml.personal." + timestr)
            if os.path.exists(conf_personal):
                print("Backup conf.xml.personal to conf.xml.personal." + timestr)
                shutil.copyfile(conf_personal, conf_personal_backup)

    def delete(self, widget):
        filename = os.path.join(self.conf_dir, self.conf_file_combo.get_active_text())
        # TODO: dialog: are you certain?
        os.remove(filename)
        self.update_label()
        self.find_conf_files()


    def accept(self, widget):
        self.backupconf()
        link_source = self.conf_file_combo.get_active_text()
        os.remove(self.conf_xml)
        os.symlink(link_source, self.conf_xml)
        self.update_label()
        self.find_conf_files()

    def personal(self, widget):
        self.backupconf(True)
        template_file = os.path.join(self.conf_dir, self.conf_file_combo.get_active_text())
        personal_file = os.path.join(self.conf_dir, "conf.xml.personal")
        shutil.copyfile(template_file, personal_file)
        os.remove(self.conf_xml)
        os.symlink("conf.xml.personal", self.conf_xml)
        self.update_label()
        self.find_conf_files()

    # Constructor Functions

    def destroy(self, widget, data=None):
        gtk.main_quit()

    def __init__(self):
        self.window = gtk.Window(gtk.WINDOW_TOPLEVEL)
        self.window.set_title("Paparazzi Configuration Chooser")

        self.my_vbox = gtk.VBox()

        # if PAPARAZZI_HOME not set, then assume the tree containing this
        # file is a reasonable substitute
        self.paparazzi_home = os.getenv("PAPARAZZI_HOME", os.path.dirname(os.path.abspath(__file__)))
        self.conf_dir = os.path.join(self.paparazzi_home, "conf")
        self.conf_xml = os.path.join(self.conf_dir, "conf.xml")

        self.exclude_backups = True

        # MenuBar
        mb = gtk.MenuBar()

        # File
        filemenu = gtk.Menu()

        # File Title
        filem = gtk.MenuItem("File")
        filem.set_submenu(filemenu)

        exitm = gtk.MenuItem("Exit")
        exitm.connect("activate", gtk.main_quit)
        filemenu.append(exitm)

        mb.append(filem)

        # Help
        helpmenu = gtk.Menu()

        # Help Title
        helpm = gtk.MenuItem("Help")
        helpm.set_submenu(helpmenu)

        aboutm = gtk.MenuItem("About")
        aboutm.connect("activate", self.about)
        helpmenu.append(aboutm)

        mb.append(helpm)

        self.my_vbox.pack_start(mb,False)

        # Combo Bar

        self.conf_label = gtk.Label("Conf:")

        self.conf_file_combo = gtk.combo_box_entry_new_text()
        self.find_conf_files()
#        self.firmwares_combo.connect("changed", self.parse_list_of_airframes)
        self.conf_file_combo.set_size_request(600,30)

        self.confbar = gtk.HBox()
        self.confbar.pack_start(self.conf_label)
        self.confbar.pack_start(self.conf_file_combo)
        self.my_vbox.pack_start(self.confbar, False)

        ##### Explain current config

        self.explain = gtk.Label("");
        self.update_label()

        self.exbar = gtk.HBox()
        self.exbar.pack_start(self.explain)

        self.my_vbox.pack_start(self.exbar, False)

        ##### Buttons
        self.btnAccept = gtk.Button("Set Selected Conf As Active")
        self.btnAccept.connect("clicked", self.accept)
        self.btnAccept.set_tooltip_text("Set Conf as Active")

        self.btnPersonal = gtk.Button("Create Personal Conf Based on Selected")
        self.btnPersonal.connect("clicked", self.personal)
        self.btnPersonal.set_tooltip_text("Create Personal Conf Based on Selected and Activate")

        self.btnDelete = gtk.Button("Delete Selected")
        self.btnDelete.connect("clicked", self.delete)
        self.btnDelete.set_tooltip_text("Permanently Delete")

        self.btnLaunch = gtk.Button("Launch Paparazzi")
        self.btnLaunch.connect("clicked", self.launch)
        self.btnLaunch.set_tooltip_text("Launch Paparazzi with current conf.xml")

        self.btnExit = gtk.Button("Exit")
        self.btnExit.connect("clicked", self.destroy)
        self.btnExit.set_tooltip_text("Close application")


        self.toolbar = gtk.HBox()
        self.toolbar.pack_start(self.btnAccept)
        self.toolbar.pack_start(self.btnPersonal)
        self.toolbar.pack_start(self.btnDelete)
        self.toolbar.pack_start(self.btnLaunch)
        self.toolbar.pack_start(self.btnExit)

        self.my_vbox.pack_start(self.toolbar, False)

        ##### Bottom

        self.window.add(self.my_vbox)
        self.window.show_all()
        self.window.set_position(gtk.WIN_POS_CENTER_ALWAYS)
        self.window.connect("destroy", self.destroy)

    def main(self):
        gtk.main()

if __name__ == "__main__":
    import sys
    if (len(sys.argv) > 1):
        airframe_file = sys.argv[1]
    gui = ConfChooser()
    gui.main()
