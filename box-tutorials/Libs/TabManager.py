# coding: utf8
import numpy
import csv
STATS_LABELS = ["Session", "Bloc", "Min", "Max", "Moyenne", "Ecart-Type", "Super Critiques", "Critiques", "Recompenses", "Super Recompenses", "Score"]
NB_BLOC = 8
Y = 3
X = 1.5

#######################################
##### Identifiant dans le tableau #####
#######################################
def ToIdx(session, bloc):
	return int((session-1)*NB_BLOC+bloc)

###############################
##### Calcul des 4 Seuils #####
###############################
def GetThres(mean,std):
	return numpy.array([max(0,mean-Y*std), max(0,mean-X*std), mean+X*std, mean+Y*std])

#################################
##### Concatenation des CSV #####
#################################
def ConcatCSV(path, files, stats, thres):
	# Creation d'un tableau contenant toutes les stats
	for f in files:
		with open(path+f, 'r') as csvfile:
			CSVReader = csv.reader(csvfile)
			for row in CSVReader:
				if(int(row[0])!=-1 and int(row[1])!=-1):
					Idx = ToIdx(int(row[0]),int(row[1]))
					stats[Idx][0] = int(row[0])		# Session
					stats[Idx][1] = int(row[1])		# Bloc
					for i in range(2,len(STATS_LABELS)):
						stats[Idx][i] = float(row[i+8])

	
	# Calcul correct des seuils
	for i in range(len(stats)):
		if(stats[i][0]!=-1 and stats[i][1]!=-1):
			T = []
			if(stats[i][1]<3):					# Calibrage, bloc 1 et 2 utilisent le calibrage comme ref
				id_base = int(i-stats[i][1])
				T = GetThres(stats[id_base][4], stats[id_base][5])
			else:
				id_base1 = i - 2
				id_base2 = i
				if(stats[i][1]%2==0):	id_base2 -= 3
				else:					id_base2 -= 1
				T1 = GetThres(stats[id_base1][4],stats[id_base1][5])
				T2 = GetThres(stats[id_base2][4],stats[id_base2][5])
				T = (T1 + T2) / 2
			thres[i] = numpy.concatenate(([stats[i][0], stats[i][1]], T))

	return stats, thres

################################
##### Ecriture dans un CSV #####
################################
def CSVWriter(filename, tab, header, excel=False):
	with open(filename, 'w') as csvfile:
		if(excel):	Writer = csv.writer(csvfile, delimiter = ';', lineterminator = '\n', quoting = csv.QUOTE_NONE)
		else:		Writer = csv.writer(csvfile, delimiter = ',', lineterminator = '\n', quoting = csv.QUOTE_NONE)
		Writer.writerow(header)
		for row in tab:
			Writer.writerow(row)


####################################################################
##### Recuperation des informations utiles en deux numpy array #####
####################################################################
def GetRatios(filename, header=True):
	Time = []
	Ratio = []
	with open(filename, 'r') as f:				# Ouverture du CSV
		R = csv.reader(f)						# Reader
		if(header):		next(R, None)			# Skip du header
		for row in R:							# Parcours des donnees
			Time.append(float(row[0]))
			Ratio.append(float(row[2]))
	return numpy.array(Time), numpy.array(Ratio)	


############################################
##### Calcul des temps sur chaque zone #####
############################################
def ComputeAreasStats(time, datas, thres, session, bloc):
	# Variables
	Count = numpy.array([0.0]*5)				# 0-min-critique-recompense-max-\infty
	Max_Time = numpy.array([0.0]*5)				# 0-min-critique-recompense-max-\infty
	Prev_Area = -1
	Time_Area = 0.0
	for i in range(len(datas)):					# Parcours des valeurs
		Area = -1								# Recuperation de l'intervalle
		if (datas[i] < thres[0]):	Area = 0	
		elif (datas[i] < thres[1]):	Area = 1	
		elif (datas[i] < thres[2]):	Area = 2	
		elif (datas[i] < thres[3]):	Area = 3	
		else :						Area = 4
		Count[Area] += 1						# Ajout au compteur de valeur
		if (Prev_Area != Area):					# Si changement d'intervalle
			Prev_Area = Area					# Changement d'intervalle de reference
			Time_Area = time[i]					# On remet a zero le compteur de temps
		Ellapsed_Time = time[i] - Time_Area	
		if (Ellapsed_Time > Max_Time[Area]):
			Max_Time[Area] = Ellapsed_Time		# Mise a jour du temps max dans l'intervalle

	Count = numpy.divide(numpy.multiply(Count,100), len(datas))
	Times = numpy.concatenate(([session, bloc], Count))
	Times_Max = numpy.concatenate(([session, bloc], Max_Time))
	return Times, Times_Max

