# coding: utf8
import os, sys, numpy, math, csv

class Configs_Save(OVBox):
	def __init__(self):
		OVBox.__init__(self)

		self.Session = 0
		self.Bloc = 0
		self.Time = 0
		self.Thresholds = numpy.zeros((4,4))
		self.Final_Thresholds = numpy.zeros((2,4))
		self.Stats = numpy.zeros((4,8))
		self.Max_A = 0.0
		self.Max_B = 0.0
		self.Reward_Time = 0.0
		self.S_Reward = 0
		self.First = True
		self.Buffer = numpy.zeros(4)
		self.Stimcode = "OVTK_StimulationId_Label_02"

	##########################
	##### Initialisation #####
	def initialize(self):
		assert len(self.input)  == 0, 'This box needs exactly 0 input'
		assert len(self.output) == 3, 'This box needs exactly 3 output'
		
		##### Récupération des settings #####
		self.Path = self.setting['Config Path']
		self.InputFilename = self.setting['Fichier Stat']
		self.Session = int(self.InputFilename[-13])
		self.Bloc = int(self.InputFilename[-11])
		print(" Session : \t" + str(self.Session) + "." + str(self.Bloc))

		self.X = float(self.setting['X'])
		self.Y = float(self.setting['Y'])
		self.Coef = float(self.setting['Coef Artéfact'])
		self.Reward_Time = float(self.setting['Duree pour recompense'])
		self.S_Reward = int(self.setting['Niveau super-recompense'])
		self.Time = int(self.setting['Durée Bloc'])

		if(self.Bloc!=0):
			PrevFile1, PrevFile2, PrevFile3 = self.GetPrevFiles(self.InputFilename)
			if (PrevFile3):
				self.Stats[3,:] = self.ReadFile(PrevFile3)
				print(" Stats B"+str(self.Bloc-3) + " : \t" + numpy.array2string(self.Stats[3,:], precision=4, separator=", "))
			if (PrevFile2):
				self.Stats[2,:] = self.ReadFile(PrevFile2)
				print(" Stats B"+str(self.Bloc-2) + " : \t" + numpy.array2string(self.Stats[2,:], precision=4, separator=", "))
			if (PrevFile1):
				self.Stats[1,:] = self.ReadFile(PrevFile1)
				print(" Stats B"+str(self.Bloc-1) + " : \t" + numpy.array2string(self.Stats[1,:], precision=4, separator=", "))

		self.Stats[0,:] = self.ReadFile(self.InputFilename)
		print(" Stats B"+str(self.Bloc) + " : \t" + numpy.array2string(self.Stats[0,:], precision=4, separator=", "))
		self.Compute()

		print(" Seuils : \t" + numpy.array2string(self.Final_Thresholds[0,:], precision=4, separator=", "))
		print(" Durée R : \t" + str(self.Reward_Time))
		print(" Niveau S R : \t" + str(self.S_Reward))
		print(" MAX A : \t" + "%.4f" % (self.Max_A))
		print(" MAX B : \t" + "%.4f" % (self.Max_B))
		print("\n Enregistrement des Fichiers Configs...")
		self.WriteConfigFile()
		print(" Fichiers Configs Enregistres")
		self.Display()

	#################################
	##### Process pour terminer #####
	def process(self):
		if (self.First):
			Chunk = OVStreamedMatrixBuffer(self.getCurrentTime(), self.getCurrentTime()+1./self.getClock(), self.Buffer.tolist())
			self.output[2].append(Chunk)
			self.First = False

		else :
			stimSet = OVStimulationSet(0, self.getCurrentTime()+1./self.getClock())
			stimSet.append(OVStimulation(OpenViBE_stimulation["OVTK_StimulationId_EndOfFile"], 0, 0.))
			self.output[0].append(stimSet)


	####################################################
	##### Récupération des Stats du bloc précédent #####
	def GetPrevFiles(self, filename):
		Dir = os.path.dirname(filename)
		Filelist = []
		# Récupération des Fichiers
		for (_, _, Files) in os.walk(Dir):
			Filelist.extend(Files)
			break

		File1 = next(f for f in Filelist if (f[-13]==str(self.Session) and f[-11]==str(self.Bloc-1)))
		if (self.Bloc > 1) :
			File2 = next(f for f in Filelist if (f[-13]==str(self.Session) and f[-11]==str(self.Bloc-2)))
		else:
			File2 = File1
		if (self.Bloc > 2) :
			File3 = next(f for f in Filelist if (f[-13]==str(self.Session) and f[-11]==str(self.Bloc-3)))
		else :
			File3 = File2
		return os.path.join(Dir, File1), os.path.join(Dir, File2), os.path.join(Dir, File3)


	###################################
	##### Lecture du fichier Stat #####
	def ReadFile(self, filename):
		with open(filename, 'r') as csvfile:
			Reader = csv.reader(csvfile)
			for row in Reader:
				stats = numpy.array([float(row[4]), float(row[5]), float(row[8]), float(row[9]), float(row[12]), float(row[13]), float(row[16]), float(row[17])])
		return stats

	##########################################################
	##### Calcul des seuils s'ils ne sont pas surchargés #####
	def Compute(self):
		A = numpy.zeros(4)
		B = numpy.zeros(4)
		for i in range(4):
			self.Thresholds[i,0] = self.Stats[i,4] - self.Y * self.Stats[i,5]
			self.Thresholds[i,1] = self.Stats[i,4] - self.X * self.Stats[i,5]
			self.Thresholds[i,2] = self.Stats[i,4] + self.X * self.Stats[i,5]
			self.Thresholds[i,3] = self.Stats[i,4] + self.Y * self.Stats[i,5]
			# Vérification de cohérence
			for j in range(4):
				if(self.Thresholds[i,j] < 0.0):
					self.Thresholds[i,j] = 0.0
			#Seuil pour les bandes fréquentielles A et B
			A[i] = self.Stats[i,0] + self.Coef * self.Stats[i,1]
			B[i] = self.Stats[i,2] + self.Coef * self.Stats[i,3]
			if(self.Bloc!=0):
				print(" Seuils B"+str(self.Bloc-i) +" : \t" + numpy.array2string(self.Thresholds[i,:], precision=4, separator=", ") + "\t MAX A : "+"%.4f" % (A[i])+"\t MAX B : "+"%.4f" % (B[i]))

		if(self.Bloc!=0):
			self.Final_Thresholds[0,:] = (self.Thresholds[0,:]+self.Thresholds[1,:]) / 2
			self.Final_Thresholds[1,:] = (self.Thresholds[2,:]+self.Thresholds[3,:]) / 2
			self.Max_A = (A[0]+A[1]) / 2
			self.Max_B = (B[0]+B[1]) / 2
		else:
			self.Final_Thresholds[0,:] = self.Thresholds[0,:]
			self.Max_A = A[0]
			self.Max_B = B[0]
		

	############################################
	##### Ecriture des fichiers de configs #####
	def WriteConfigFile(self):
		strBegin = "<OpenViBE-SettingsOverride>\n"
		strEnd = "</OpenViBE-SettingsOverride>"

		# Display Config
		self.fileConfig = open(self.Path+"Display.xml", 'w')
		strToWrite = strBegin																	# Balise de debut
		strToWrite += "\t<SettingValue>0:0,0,0; 100:100,100,100</SettingValue>\n"				# Gradient Color
		strToWrite += "\t<SettingValue>255</SettingValue>\n"									# Gradient Step
		strToWrite += "\t<SettingValue>"+str(self.Final_Thresholds[0,0])+"</SettingValue>\n"	# Min
		strToWrite += "\t<SettingValue>"+str(self.Final_Thresholds[0,3])+"</SettingValue>\n"	# Max
		strToWrite += "\t<SettingValue>"+str(self.Time+7)+"</SettingValue>\n"					# Durée d'un bloc + 7 pour les pauses avant/après)
		strToWrite += strEnd																	# Balise de Fin
		self.fileConfig.write(strToWrite)
		self.fileConfig.close()
		print("\tDisplay.xml enregistre")

		# Score Config
		self.fileConfig = open(self.Path+"Score.xml", 'w')
		strToWrite = strBegin																	# Balise de debut
		strToWrite += "\t<SettingValue>"+str(self.Reward_Time)+"</SettingValue>\n"				# Duree recompense
		strToWrite += "\t<SettingValue>"+str(self.S_Reward)+"</SettingValue>\n"					# Niveau super-recompense
		strToWrite += "\t<SettingValue>"+str(self.Final_Thresholds[0,0])+"</SettingValue>\n"	# Min
		strToWrite += "\t<SettingValue>"+str(self.Final_Thresholds[0,1])+"</SettingValue>\n"	# Critique
		strToWrite += "\t<SettingValue>"+str(self.Final_Thresholds[0,2])+"</SettingValue>\n"	# Recompense
		strToWrite += "\t<SettingValue>"+str(self.Final_Thresholds[0,3])+"</SettingValue>\n"	# Max
		strToWrite += strEnd																	# Balise de Fin
		self.fileConfig.write(strToWrite)
		self.fileConfig.close()
		print("\tScore.xml enregistre")

		# Reject Config
		self.fileConfig = open(self.Path+"Higher_Energy_Reject_A.xml", 'w')
		strToWrite = strBegin																	# Balise de debut
		strToWrite += "\t<SettingValue>((A &gt; " + str(self.Max_A) + " || A &lt; 0.1) || B == 1) ? 0 : A</SettingValue>\n"	# formule
		strToWrite += strEnd																	# Balise de Fin
		self.fileConfig.write(strToWrite)
		self.fileConfig.close()
		print("\tHigher_Energy_Reject_A.xml enregistre")

		self.fileConfig = open(self.Path+"Higher_Energy_Reject_B.xml", 'w')
		strToWrite = strBegin																	# Balise de debut
		strToWrite += "\t<SettingValue>((A &gt; " + str(self.Max_B) + " || A &lt; 0.1) || B == 1) ? 0 : A</SettingValue>\n"	# formule
		strToWrite += strEnd																	# Balise de Fin
		self.fileConfig.write(strToWrite)
		self.fileConfig.close()
		print("\tHigher_Energy_Reject_B.xml enregistre")

	###################################
	##### Sorties pour Affichages #####
	def Display(self):
		if(self.Bloc!=0):
			self.UpdateBuffer()

		stimSet = OVStimulationSet(0, 0)
		stimSet.append(OVStimulation(OpenViBE_stimulation[self.Stimcode], 0, 0.))
		self.output[1].append(stimSet)

		sHeader = OVStreamedMatrixHeader( 0, 0, [2,2], ["Recompenses", "Super Recompenses", "Bloc "+str(self.Bloc-1), "Bloc "+str(self.Bloc)])
		self.output[2].append(sHeader)


	#################################
	##### Mise à Jour du Buffer #####
	def UpdateBuffer(self):
		self.Buffer[0] = self.Stats[1,-2]	# Recompense Bloc -1 
		self.Buffer[2] = self.Stats[1,-1]	# Super Recompense Bloc -1
		self.Buffer[1] = self.Stats[0,-2]	# Recompense Bloc 
		self.Buffer[3] = self.Stats[0,-1]	# Super Recompense Bloc
		
		print(" Prev Thres = " + "%.4f" % (self.Final_Thresholds[1,2]) + "\tCurr Thres = " + "%.4f" % (self.Final_Thresholds[0,2]))
		print(" Changement = "+"%.2f"% (((self.Final_Thresholds[0,2]/self.Final_Thresholds[1,2])-1)*100)+" %")
		if(self.Final_Thresholds[0,2] > self.Final_Thresholds[1,2]):	self.Stimcode = "OVTK_StimulationId_Label_01"
		elif(self.Final_Thresholds[0,2] < self.Final_Thresholds[1,2]):	self.Stimcode = "OVTK_StimulationId_Label_03"



box = Configs_Save()