#!/usr/bin/python
# vim: set fileencoding=utf-8

from gi.repository import Gtk
import subprocess
from os.path import expanduser

home = expanduser("~")

class AnnotaterWindow(Gtk.Window):

	def launchAnnotater(self, button):
		if self.filename:
			subprocess.Popen([home+"/BA/testSampleAnnotater/Debug/TestSampleAnnotater", self.filename])

	def chooseFile(self, widget):
		dialog = Gtk.FileChooserDialog("Please choose a file", self,
			Gtk.FileChooserAction.OPEN,
			(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL,
			 Gtk.STOCK_OPEN, Gtk.ResponseType.OK))

		if self.currentFolder:
			dialog.set_current_folder(self.currentFolder)

		filter_yml = Gtk.FileFilter()
		filter_yml.set_name("YML persisted opencv matrices")
		filter_yml.add_mime_type("text/yaml")
		dialog.add_filter(filter_yml)

		filter_png = Gtk.FileFilter()
		filter_png.set_name("PNG image files")
		filter_png.add_mime_type("image/png")
		dialog.add_filter(filter_png)


		response = dialog.run()

		if response == Gtk.ResponseType.OK:
			self.filename = dialog.get_filename()
			self.currentFolder = self.filename[:self.filename.rfind('/')+1]
			self.startButton.set_label("Starte annotieren von "+self.filename[self.filename.rfind('/')+1:])
			self.startButton.set_sensitive(True)

		elif response == Gtk.ResponseType.CANCEL:
			self.filename = None
			self.startButton.set_label("Wähle zunächst eine Datei")
			self.startButton.set_sensitive(False)

		dialog.destroy()

	def keypress_event(self, widget, event):
		if event.keyval == 65307: #ESC-Key
			Gtk.main_quit()

	def __init__(self):
		Gtk.Window.__init__(self, title="Sample Annotater")
		self.filename = None
		self.currentFolder = None

		self.set_border_width(10)
		self.connect('key_press_event', self.keypress_event)

		vbox = Gtk.Box(orientation=Gtk.Orientation.VERTICAL)
		hbox = Gtk.Box()

		introText = Gtk.Label("""① Bitte setze auf jede Kreuzung ohne Stein einen grauen Kreis.
② Bitte setze auf jede Kreuzung, die von einem Stein verdeckt wird einen Kreis in der Farbe des Steines (zB weißer Stein => weißer Kreis).
WICHTIG: Setze den Kreis dorthin, wo die Kreuzung ist, nicht dort, wo die Mitte des Steines ist
③ Schließe das Fenster mit ESC """)
		introText.set_line_wrap(True)

		choooseButton = Gtk.Button("Wähle eine Datei")
		choooseButton.connect("clicked", self.chooseFile)

		self.startButton = Gtk.Button("Wähle zunächst eine Datei", sensitive=False)
		self.startButton.connect("clicked", self.launchAnnotater)

		self.add(vbox)
		vbox.pack_start(introText, True, True, 0)
		vbox.pack_start(hbox, True, True, 0)
		hbox.pack_start(choooseButton, True, True, 0)
		hbox.pack_start(self.startButton, True, True, 0)

win = AnnotaterWindow()
win.connect("delete-event", Gtk.main_quit)
win.show_all()
Gtk.main()
