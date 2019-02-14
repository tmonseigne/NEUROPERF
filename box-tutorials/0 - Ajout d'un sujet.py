# coding: utf8
import os

###############################
########## Variables ##########
###############################
# Path & Files
Path = ""
Dirs = ["Stats/","Data/","Data/Signal/","Raw/","Report/","Report/img/"]

# Datas
ID_Sujet = ""

###############################
########## Fonctions ##########
###############################
# Selection du sujet
def SubjectSelection():
	global Path, ID_Sujet

	while (not ID_Sujet):
		ID_Sujet = raw_input("Entrez l\'identifiant du sujet : ")
		#if (ID_Sujet and os.path.isdir(Path+ID_Sujet+"/")):
		#	print("Sujet deja existant")
		#	ID_Sujet = ""
	
	Path += ID_Sujet + "/"
###################################################################################################


Path = os.path.join(os.path.dirname(__file__), "../Enregistrements/")
if (not os.path.isdir(Path)):
	print("Dossier \'../Enregistrements/\' Introuvable...")
	raw_input('Appuyez sur entree pour terminer...')
	exit(1)

#Selection du sujet
SubjectSelection()
for i in range(len(Dirs)):
	if (not os.path.isdir(Path+Dirs[i])):
		os.makedirs(Path+Dirs[i])

raw_input('Appuyez sur entree pour terminer...')
