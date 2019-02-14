# coding: utf8
import numpy
import math
import csv

class Stats_Save(OVBox):
	def __init__(self):
		OVBox.__init__(self)
		# Session, Bloc, 
		# Min, Max, Moyenne, Ecart-Type pour A
		# Min, Max, Moyenne, Ecart-Type pour B
		# Min, Max, Moyenne, Ecart-Type pour A/B
		# Super Critiques, Critiques, Recompenses, Super Recompenses
		self.Stats = [[-1.0]*2,[-1.0]*4,[-1.0]*4,[-1.0]*4,[-1.0]*5]

	##########################
	##### Initialisation #####
	def initialize(self):
		assert len(self.input)  == 4, 'This box needs exactly 4 input'
		assert len(self.output) == 0, 'This box needs exactly 0 output'
		
		##### Récupération des settings #####
		self.filenameStats = self.setting['Output Stats Filename']
		self.Stats[0][0]  = int(self.setting['Session'])
		self.Stats[0][1]  = int(self.setting['Bloc'])

	##################################################
	##### Ecriture du fichier seulement à la fin #####
	def uninitialize(self):
		self.WriteStatsFile()

	###################
	##### Process #####
	def process(self):
		for i in range(4):
			self.FillStats(i)
		self.WriteStatsFile()

	###########################################
	##### Remplissage du tableau de Stats #####
	def FillStats(self, k):
		while self.input[k]:
			chunk = self.input[k].pop()									# Recuperation de l'entree k
			if(type(chunk) == OVStreamedMatrixBuffer):					# Buffer Reçu
				s = k + 1
				for i in range(len(self.Stats[s])):
					self.Stats[s][i] = float(chunk[i])	

	##############################
	##### Ecriture des Stats #####
	def WriteStatsFile(self):
		with open(self.filenameStats, 'w') as csvfile:
			CSVWriter = csv.writer(csvfile, delimiter=',', lineterminator='\n', quoting=csv.QUOTE_NONE)
			#CSVWriter.writerow(["Session", "Bloc", ("Min", "Max", "Moyenne", "Ecart-Type")*3, "Super Critiques", "Critiques", "Recompenses", "Super Recompenses"])
			CSVWriter.writerow(sum(self.Stats,[]))


box = Stats_Save()