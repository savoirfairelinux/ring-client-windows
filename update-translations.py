#!/usr/bin/python

import os
import shutil

print ("Updating translations")

os.system("tx pull -af --minimum-perc=30")

print ("Updating .pro file")

translationFiles = []

for filename in os.listdir('./translations'):
    translationFiles.append("translations/{0}".format(filename))

proFile = "RingWinClient.pro"
shutil.move(proFile, proFile + "~")

destination = open(proFile, "w")
source = open(proFile + "~", "r")
for line in source:
    if not ".ts" in line:
        destination.write(line)
    if "TRANSLATIONS = " in line:
        for filename in translationFiles:
            destination.write("filename \\\n")

source.close()
destination.close()
os.remove(proFile + "~")
