# coding: utf8
import os
import sys
import numpy
import math
import csv
import markdown
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
sys.path.append(os.path.abspath(os.path.join(__file__,"..","Libs")))
import FileManager as fm
import TabManager as tm
import FigureManager as fgm

###############################
########## Variables ##########
###############################
# Const
STATS_LABELS = ["Session", "Bloc", "Min", "Max", "Moyenne", "Ecart-Type", "Super Critiques", "Critiques", "Recompenses", "Super Recompenses", "Score"]
TIME_AREA_LABELS = ["Session", "Bloc", "0 - Min", "Min - Critique", "Critique - Récompense", "Récompense - Max", "Max - +&infin;"]
THRESHOLDS_LABELS = ["Session", "Bloc", "Min", "Critique", "Récompense", "Max"]
NB_SESSION = 8
NB_BLOC = 8
TOTAL_BLOC = NB_SESSION * NB_BLOC

# Path & Files
_Path = _PathStat = _PathData = _PathRepo = ""
_FilesStat = _FilesRatio = []
_Dates = [""] * NB_SESSION
_Sujets = []
_ID_Sujet = ""
_Sessions = []
# Datas
_Stats = numpy.zeros((TOTAL_BLOC, len(STATS_LABELS)))					# Stats & Récompense & Score
_Thresholds = numpy.zeros((TOTAL_BLOC, len(THRESHOLDS_LABELS))) - 1		# Thresholds
_MetaStat = numpy.empty((0,len(STATS_LABELS)), float)					# Meta Stats
_Meta_Thres = numpy.empty((0,2), float)									# Seuils critique et récompense
_Times_Areas = numpy.zeros((TOTAL_BLOC, len(TIME_AREA_LABELS))) - 1		# Temps dans chaque section en pourcent
_Times_Areas_Max = numpy.zeros((TOTAL_BLOC, len(TIME_AREA_LABELS))) - 1	# Temps Max dans chaque section
###############################
###############################
###############################

###############################
########## Fonctions ##########
###############################
# Récupération de l'indice
def ToIdx(session, bloc):
	return int((session-1)*NB_BLOC+bloc)
###################################################################################################

# Réinitialise les variables
def ResetVar():
	global _FilesStat, _FilesRatio, _Dates, _Thresholds, _Stats, _Times_Areas, _Times_Areas_Max, _MetaStat, _Meta_Thres
	_FilesStat = []
	_FilesRatio = []
	_Dates = [""] * NB_SESSION	
	_Stats = numpy.zeros((TOTAL_BLOC, len(STATS_LABELS))) - 1				# Stats
	_Thresholds = numpy.zeros((TOTAL_BLOC, len(THRESHOLDS_LABELS))) - 1		# Thresholds
	_Times_Areas = numpy.zeros((TOTAL_BLOC, len(TIME_AREA_LABELS))) - 1		# Temps dans chaque section en pourcent
	_Times_Areas_Max = numpy.zeros((TOTAL_BLOC, len(TIME_AREA_LABELS))) - 1	# Temps Max dans chaque section
	_MetaStat = numpy.empty((0,len(STATS_LABELS)), float)					# Idem que les Stats sauf les blocs
	_Meta_Thres = numpy.empty((0,2), float)									# Seuils critique et récompense
###################################################################################################

# Met a jour les chemins
def PathUpdates():
	global _Path, _PathStat, _PathData, _PathRepo
	ResetVar()
	_Path = os.path.join(os.path.dirname(__file__), "../Enregistrements/") + _ID_Sujet + "/"
	_PathStat = _Path + "Stats/"
	_PathData = _Path + "Data/"
	_PathRepo = _Path + "Report/"
###################################################################################################

# Selection du sujet
def SubjectSelection():
	global _ID_Sujet, _Sujets
	_Sujets = fm.GetDirList(_Path)									# Parcours des Dossiers

	if (not _Sujets):												# Aucun Sujet
		print ("Aucun Sujet")	
		exit(1)

	while (not _ID_Sujet):
		print ("Liste des Sujets : " + ",".join(_Sujets))
		_ID_Sujet = raw_input("Entrez l\'identifiant du sujet (vide pour tous): ")
		if (not _ID_Sujet):											# Tous les rapports
			_ID_Sujet = "all"						
		elif (not any(x in _ID_Sujet for x in _Sujets)):
			print("Sujet Inconnu")
			_ID_Sujet = ""
###################################################################################################

# Selection du sujet
def SessionSelection():
	global _Sessions
	_Sessions = numpy.array(range(8))+1								# liste des sessions

	text = raw_input("Entrez le numero de la session (vide pour toutes): ")
	if (text and 0 < int(text) and int(text)<=8):
		_Sessions = [int(text)]						
###################################################################################################

# Recuperation des listes de fichiers
def GetFileList():
	global _FilesStat, _FilesRatio
	# Récupération des Fichiers
	_FilesStat = fm.GetFileList(_PathStat)
	_FilesRatio = fm.GetFileList(_PathData)

	#Au cas ou suppression des fichiers non conforme
	_FilesStat = fm.CleanFileList(_FilesStat, "Stat")
	_FilesRatio = fm.CleanFileList(_FilesRatio, "Ratio")

	if(_FilesStat):
		sys.stdout.write("Liste des Sessions : ")
		for f in _FilesRatio:
			_, Session, Bloc, _, _ = fm.GetInfos(f)
			sys.stdout.write("\t"+str(Session) + "."+str(Bloc))
		sys.stdout.write("\n")
	else:
		print("Aucun Fichier")
###################################################################################################

# Création des figures de données
def MakeRatioFigures():
	global _Times_Areas, _Times_Areas_Max, _Dates										# Récupération des variables globales pour modification
	print ("Enregistrement des Figures des donnees...")
	LABELS = ["Puissance","Min","Critique","Recompense","Max"]
	fgm.LegendFig(LABELS, ["C0", "C3", "C1", "C4", "C2"], ["-","--","--","--","--"], _PathRepo + "img/Legende.png")
	for f in _FilesRatio:
		Date, Session, Bloc, _, _ = fm.GetInfos(f)
		_Dates[Session-1] = Date
		if (Session in _Sessions):
			Time, Ratio = tm.GetRatios(_PathData+f)
			if (len(Time) != 0 and len(Ratio) != 0):									# Verification List non vide 
				sys.stdout.write("\tSession " + str(Session) +" Bloc "+ str(Bloc) + "\t")
				Thres = fgm.GetThresholdsForFigure(Session, Bloc, NB_BLOC, _Thresholds)	# Recuperation des seuils si ils existent
				im_name = str(Session) +"_"+ str(Bloc)+"_Ratio.png"						# Nom du fichier
				fgm.RatioFig(Time, Ratio, Thres, LABELS, Bloc, _PathRepo + "img/" + im_name, False)						# Figure des donnees
				sys.stdout.write("Ratio OK\t")
				fgm.HistFig(Ratio, Thres, LABELS, Bloc, _PathRepo + "img/" + im_name.replace("Ratio","Hist"), False)	# Histogramme des donnees
				sys.stdout.write("Hist OK\n")
				if (Bloc != 0 and len(Thres)!=0):	
					Idx = ToIdx(Session, Bloc)											# Id du bloc
					_Times_Areas[Idx], _Times_Areas_Max[Idx] = tm.ComputeAreasStats(Time, Ratio, Thres, Session, Bloc)
	print ("Enregistrement des Figures des donnees OK")

###################################################################################################

# Création des Figures de Stats
def MakeStatsFigures():
	global _MetaStat, _Meta_Thres
	LABELS = ["Distribution","Min","Max", "Super Critiques", "Critiques", "Recompenses", "Super Recompenses"]
	print ("Enregistrement des Figures Statistiques...")
	for i in range(NB_SESSION):									# Pour chaque session
		Idx = i * NB_BLOC
		im_name = _PathRepo + "img/" + str(i+1)+"_Stats.png"
		if(_Stats[Idx][0]==i+1):
			last_bloc = 0
			sys.stdout.write("\tSession " + str(i+1) + "\t")
			S_Stats = numpy.array([_Stats[Idx][2:]])			# Récupération de toutes les colonnes sauf les deux premières de la ligne Idx 
			S_Thres = numpy.empty((0,2), float)					# Seuils critique et récompense
			for j in range(Idx+1,Idx+NB_BLOC):					# Récupération des valeurs utiles
				if(_Stats[j][0]==i+1):
					S_Stats = numpy.append(S_Stats, [_Stats[j][2:]], axis=0)
					S_Thres = numpy.append(S_Thres, [_Thresholds[j][3:5]], axis=0)
					last_bloc=int(_Stats[j][1])

			X_axis = range(S_Stats.shape[0])
			fgm.StatFig(X_axis, S_Stats, LABELS, "Bloc", im_name, False)
			sys.stdout.write("Stats OK\t")
			X_axis=X_axis[1:]
			fgm.RewardFig(X_axis, S_Stats[1:,], LABELS, "Bloc", im_name.replace("Stats","Recompenses"))
			sys.stdout.write("Recompenses OK\t")
			fgm.ScoreFig(X_axis, S_Stats[1:,], "Bloc", im_name.replace("Stats","Scores"), False)
			sys.stdout.write("Scores OK\t")
			fgm.ThresFig(X_axis, S_Thres, "Bloc", im_name.replace("Stats","Seuils"), False, False)
			sys.stdout.write("Seuils OK\n")
			_MetaStat = numpy.append(_MetaStat, [numpy.append([i+1, last_bloc],numpy.mean(S_Stats[1:,:], axis=0))], axis=0)
			_Meta_Thres = numpy.append(_Meta_Thres, [numpy.mean(S_Thres[::2,:], axis=0)], axis=0)

	X_axis = numpy.array(range(_MetaStat.shape[0]))+1
	fgm.StatFig(X_axis, _MetaStat[:,2:], LABELS, "Session", _PathRepo + "img/Stats.png")
	print ("\tMeta Stats OK")
	fgm.RewardFig(X_axis, _MetaStat[:,2:], LABELS, "Session", _PathRepo + "img/Recompenses.png")
	print ("\tMeta Recompenses OK")
	fgm.ScoreFig(X_axis, _MetaStat[:,2:], "Session", _PathRepo + "img/Scores.png", False)
	print ("\tMeta Scores OK")
	fgm.ThresFig(X_axis, _Meta_Thres, "Session", _PathRepo + "img/Seuils.png",  False, False)
	print ("\tMeta Seuils OK")
	print ("Enregistrement des Figures Statistiques OK")
###################################################################################################

# Récupération d'un titre de tableau
def GetTitleRow(titles):
	row = "| "
	sep = "|"
	for i in range(len(titles)):
		row += titles[i] + " | "
		#sep += ":---:| "
		sep += ":"+"-" * (len(titles[i]))+":|"
	return row + "\n" + sep + "\n" 
###################################################################################################

# Récupération d'une ligne pour les tableaux de Stat
def GetStatsRow(stats):
	row = "| "
	for i in range(len(stats)):
		if (stats[i] != -1):
			if (i in (2,3,4,5)):
				row += '%.3f' % stats[i]
			else:
				row += str(int(stats[i]))
		row += " | "
	return row + "\n"
###################################################################################################

# Récupération d'une ligne pour les tableaux de temps
def GetTimeAreaRow(time_area, suffixe=""):
	row = "| "
	for i in range(len(time_area)):
		if (i in (0, 1)):
			row += str(int(time_area[i]))
		else:
			row += "%.3f" % time_area[i] + suffixe
		row += " | "
	return row + "\n"
###################################################################################################

# Ecriture d'une session
def WriteSession(f, s_num):
	idx = s_num * NB_BLOC
	if(_Stats[idx][0] == s_num+1):															# Si la session à au moins eu le calibrage
		f.write("<details><summary><h2>")													# Balise details et summary début
		f.write("Session " + str(s_num+1) + " "+_Dates[s_num]+" : ")						# Titre Session
		f.write("(cliquer pour afficher/cacher)</h2></summary>\n\n")						# Balise summary fini
		
		##### Données #####
		f.write("<details><summary><h3>")													# Balise details et summary début
		f.write("Données : ")																# Titre Session
		f.write("(cliquer pour afficher/cacher)</h3></summary>\n\n")
		f.write("\n::: figure-table\n\n")
		if(os.path.isfile(_PathRepo + "img/Legende.png")):
			f.write("**Légende** : ![Légende](img/Legende.png){.Legende}\n\n")
		img_row = ""
		f.write("| |\n|:---:|\n")
		for j in range(NB_BLOC):															# Pour chaque bloc
			prefixe = str(s_num+1)+"_"+str(j)
			if(os.path.isfile(_PathRepo + "img/" + prefixe+"_Ratio.png")):
				img_row += ("|![Ratio Bloc "+str(j)+"](" + "img/" + prefixe+"_Ratio.png)| ")	# Graphique des données
			else:
				img_row += "| | "
			f.write(img_row+"\n")															# Légende tout les deux
			img_row = ""
		f.write("\n")

		f.write("\n:::\n\n")
		f.write("\n</details>\n\n")


		##### Histogrammes #####
		f.write("<details><summary><h3>")													# Balise details et summary début
		f.write("Histogrammes : ")															# Titre Session
		f.write("(cliquer pour afficher/cacher)</h3></summary>\n\n")
		f.write("\n::: figure-table\n\n")
		img_row = ""
		f.write("\n| | | |\n|:---:|:---:|:---:|\n")
		for j in range(NB_BLOC):															# Pour chaque bloc
			prefixe = str(s_num+1)+"_"+str(j)
			if(os.path.isfile(_PathRepo + "img/" + prefixe+"_Hist.png")):
				img_row += ("|![Hist Bloc "+str(j)+"](" + "img/" + prefixe+"_Hist.png)| ")	# Histogramme des données
			else:
				img_row += "| | "
			if(j%2 == 1):
				if(img_row != "| | | | "):
					f.write(img_row+"\n")													# Légende tout les deux
				img_row = ""

		f.write("\n:::\n\n")
		f.write("\n</details>\n\n")

		##### Stats #####
		f.write("<details><summary><h3>")													# Balise details et summary début
		f.write("Stats : ")																	# Titre Session
		f.write("(cliquer pour afficher/cacher)</h3></summary>\n\n")
		f.write(GetTitleRow(STATS_LABELS))
		for j in range(idx,idx+NB_BLOC):
			if(_Stats[j][0] == s_num+1):
				f.write(GetStatsRow(_Stats[j]))
		
		f.write("\n::: figure-table\n\n")
		f.write("| | | |\n|:---:|:---:|:---:|\n")
		f.write("|![Stats Session " + str(s_num+1) + "]("  + "img/" + str(s_num+1) + "_Stats.png)| ")				# Graphique Statistique
		f.write("|![Seuils Session " + str(s_num+1) + "]("  + "img/" + str(s_num+1) + "_Seuils.png)|\n")			# Graphique Seuils
		#f.write("| Stats Session " + str(s_num+1) + " | | Recompenses Session " + str(s_num+1) + " |\n")			# Legende

		f.write("|![Scores Session " + str(s_num+1) + "]("  + "img/" + str(s_num+1) + "_Scores.png)| ")				# Graphique Score
		f.write("|![Recompenses Session " + str(s_num+1) + "]("  + "img/" + str(s_num+1) + "_Recompenses.png)|\n")	# Graphique Récompense
		#f.write("| Scores Session " + str(s_num+1) + " | | Seuils Session " + str(s_num+1) + " |\n")				# Legende

		f.write("\n:::\n\n")
		f.write("\n</details>\n\n")

		f.write("<details><summary><h3>")													# Balise details et summary début
		f.write("Temps : ")																	# Titre Session
		f.write("(cliquer pour afficher/cacher)</h3></summary>\n\n")

		f.write("\n#### Temps passé dans chaque section (en %) : \n\n")
		f.write(GetTitleRow(TIME_AREA_LABELS))
		for j in range(idx,idx+NB_BLOC):
			if(_Times_Areas[j][0] == s_num+1):
				f.write(GetTimeAreaRow(_Times_Areas[j], " %"))

		f.write("\n#### Temps maximum passé dans chaque section (en s) : \n\n")
		f.write(GetTitleRow(TIME_AREA_LABELS))
		for j in range(idx,idx+NB_BLOC):
			if(_Times_Areas[j][0] == s_num+1):
				f.write(GetTimeAreaRow(_Times_Areas_Max[j]," s"))

		f.write("\n</details>\n\n")
		
		f.write("\n</details>\n\n")
	#else:	print ("_Stats["str(idx)"][0] == "+str(_Stats[idx][0])+ " "+str(s_num+1)+" attendu")

###################################################################################################

# Ecriture du Résumé
def WriteSumary(f):
	if(_Stats[0][0] == 1):											# Si le premier calibrage à eu lieu
		f.write("<details><summary><h2>")							# Balise details et summary début
		f.write("Résumé : ")										# Résumé 
		f.write("(cliquer pour afficher/cacher)</h2></summary>")	# Balise summary fini
		f.write("\n\n### Stats : \n\n")
		labels = STATS_LABELS[:]
		labels.remove("Bloc")
		f.write(GetTitleRow(STATS_LABELS))							# Tableau
		for j in range(len(_MetaStat)):
			f.write(GetStatsRow(_MetaStat[j]))

		f.write("\n::: figure-table\n\n")
		f.write("| | | |\n|:---:|:---:|:---:|\n")
		f.write("|![Stats Sessions](img/Stats.png)| ")				# Graphique Statistique
		f.write("|![Seuils Sessions](img/Seuils.png)|\n")			# Graphique Récompense
		#f.write("| Stats Sessions | | Recompenses Sessions |\n")	# Legende
		f.write("|![Scores Sessions](img/Scores.png)| ")			# Graphique Statistique
		f.write("|![Recompenses Sessions](img/Recompenses.png)|\n")	# Graphique Récompense
		#f.write("| Scores Sessions | | Seuils Sessions |\n")		# Legende
		f.write("\n:::")
		f.write("\n</details>\n\n")
	#else:	print("_Stats[0][0] == "+str(_Stats[0][0])+" 1 attendu")
###################################################################################################

# Ecriture du Markdown
def WriteMarkdownFile():
	f = open(_PathRepo+"Report.md","w") 
	f.write("---\ntitle: Rapport Sujet "+_ID_Sujet+"\n---\n\n")				# Titre HTML
	f.write("# Rapport Sujet "+_ID_Sujet+" ")								# Titre Normal
	f.write("[![SANPSY](../../../Scenarios/Report/SANPSY.png){.Logo}](../../Report.html)\n\n")	# Logo
	for i in range(NB_SESSION):												# Pour chaque session
		WriteSession(f,i)													# Enregistrement de la session
	WriteSumary(f)															# Enregistrement du Résumé
	f.close()
###################################################################################################

# Pipeline de création d'un rapport
def Pipeline():
	global _Stats, _Thresholds
	print("********** "+_ID_Sujet+" **********")
	# MAJ des Chemins et vérification
	PathUpdates()
	if (not os.path.isdir(_PathStat) or not os.path.isdir(_PathData) or not os.path.isdir(_PathRepo)):
		print(_Path + "\n" + _PathStat + "\n" + _PathData + "\n"+_PathRepo)
		raw_input("Dossiers du sujet \'" + _ID_Sujet + "\' Invalides...")
		exit(1)

	# Récupération des fichiers
	GetFileList()

	# Concaténation des CSV et création des figures
	if (_FilesRatio):
		_Stats, _Thresholds = tm.ConcatCSV(_PathStat, _FilesStat, _Stats, _Thresholds)
		# Enregistrement au format CSV classique et excel
		tm.CSVWriter(_PathRepo+"Stats.csv", _Stats, STATS_LABELS, False)
		tm.CSVWriter(_PathRepo+"Stats_Excel.csv", _Stats, STATS_LABELS, True)
		MakeRatioFigures()
		MakeStatsFigures()

	# Ecriture du Markdown
	WriteMarkdownFile()
	print ("Enregistrement du Rapport Markdown OK")

	# Transformation en HTML
	Pandoc_Command = "pandoc -s " + _PathRepo + "Report.md -c ../../../Scenarios/Report/style.css -o " + _PathRepo + "Report.html --ascii --columns=10000"
	os.system(Pandoc_Command)
	print ("Enregistrement du Rapport HTML OK")
	print("**********************************\n")

###################################################################################################

# Page principale avec la liste des sujets
def MainReport():
	global _Path
	_Path = os.path.join(os.path.dirname(__file__), "../Enregistrements/")
	f = open(_Path+"Report.md","w") 
	f.write("---\ntitle: Listes des Sujets \n---\n\n")				# Titre HTML
	f.write("# Liste des Sujets ")									# Titre Normal
	f.write("![SANPSY](../Scenarios/Report/SANPSY.png){.Logo}\n\n")	# Logo
	for s in _Sujets:												# Liste des sujets
		f.write("- [&ensp;"+s+"&ensp;](./"+s+"/Report/Report.html)\n")
	f.close()
	Pandoc_Command = "pandoc -s " + _Path + "Report.md -c ../Scenarios/Report/style.css -o " + _Path + "Report.html --ascii --columns=10000"
	os.system(Pandoc_Command)
###################################################################################################

############################
########## Script ##########
############################
# Chemin
_Path = os.path.join(os.path.dirname(__file__), "../Enregistrements/")
if (not os.path.isdir(_Path)):
	raw_input("Dossier \'../Enregistrements/\' Introuvable...")
	exit(1)

SubjectSelection()			# Selection du sujet
SessionSelection()			# Selection de la session
if(_ID_Sujet=="all"):		# Si on les fait tous
	for x in _Sujets:		# Pour chaque sujet
		_ID_Sujet = x
		Pipeline()			# Création du rapport
else:
	Pipeline()

MainReport()				# Page Principale avec la liste des sujets 

raw_input('Appuyez sur entree pour terminer...')
os.startfile(_Path+"Report.html")