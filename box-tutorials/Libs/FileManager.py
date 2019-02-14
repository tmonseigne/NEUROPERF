# coding: utf8
import os, sys

#########################################
##### Change l'extension du fichier #####
#########################################
def ChangeExt(name, ext):
	return (os.path.splitext(name)[0]+ext)

##########################################
##### Récupère la liste des dossiers #####
##########################################
def GetDirList(path):
	Res = []
	# Récupération des Fichiers
	for (_, Dirs, _) in os.walk(path):
		Res.extend(Dirs)
		break
	return Res

##########################################
##### Récupère la liste des fichiers #####
##########################################
def GetFileList(path):
	Res = []
	# Récupération des Fichiers
	for (_, _, Files) in os.walk(path):
		Res.extend(Files)
		break
	return Res

#########################################
##### Nettoie la liste des fichiers #####
#########################################
def CleanFileList(filelist, name):
	return [f for f in filelist if (name in f)]

################################################
##### Récupère les informations du fichier #####
################################################
def GetInfos(filename):
	F = os.path.basename(filename)
	FSplit = F.split('_')
	if (len(FSplit)!=4) or (len(FSplit[0])<12) or (len(FSplit[1])!=1) or (len(FSplit[2])!=1):
		print("Format du fichier " + F + " invalide")
		return "",0,0,"",""
	Date = FSplit[0][1:11]
	Session = FSplit[1]
	Bloc = FSplit[2]
	Type, Ext = os.path.splitext(FSplit[3])
	return Date, int(Session), int(Bloc), Type, Ext