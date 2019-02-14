
# coding: utf8
import numpy
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker

COLORS = ["C0", "C3", "C1", "C4", "C2"]
PHYSIO_LABEL = "Ratio Beta/Alpha-Theta en Cz"
DPI = 150
FIG_SIZE = (8.10, 5.40)
FIG_LONG =  (19.20, 3.60)
X_MIN=5
X_MAX=305
Y_MIN=-0.1
Y_MAX=4
GRID_LINESTYLE = (0, (2, 6))	# loosely dotted

###############################################################################
##### Mini Figure de légende (afin d'éviter la duplication de la légende) #####
###############################################################################
def LegendFig(labels, colors, linestyles, im_name):
	Fig = plt.figure(figsize = FIG_LONG, dpi = DPI)
	for i in range(len(labels)):
		Fig.gca().plot(range(1), numpy.random.randn(1), label = labels[i], color=colors[i], linestyle=linestyles[i])
	Fig_L = plt.figure(figsize = FIG_LONG, dpi = DPI)
	Legend = plt.figlegend(*Fig.gca().get_legend_handles_labels(), loc = 'center', ncol=len(labels))
	Fig_L.canvas.draw()
	Fig_L.savefig(im_name, bbox_inches=Legend.get_window_extent().transformed(Fig_L.dpi_scale_trans.inverted()))
	plt.close('all')

######################################################################
##### Recuperation de la liste de seuil correspondant au fichier #####
######################################################################
def GetThresholdsForFigure(session, bloc, nb_bloc, thres):
	if(bloc!=0):
		Idx = int(session - 1)*nb_bloc + int(bloc)
		if(thres[Idx][0]!=-1 and thres[Idx][1]!=-1):
			return thres[Idx][2:]
	return numpy.array([], dtype = float)

###########################################
##### Création des figures de données #####
###########################################
def RatioFig(time, ratio, thres, labels, bloc, im_name, legend=True):
	Fig = plt.figure(figsize = FIG_LONG, dpi = DPI)
	Ax = plt.subplot(111)
	Ax.plot(time, ratio, label = labels[0], color=COLORS[0])
	for i in range(len(thres)):
		Ax.plot(time, numpy.array([thres[i] for j in xrange(len(time))]), label = labels[i+1], linestyle="--", color=COLORS[i+1])
	plt.xlim(X_MIN,X_MAX)
	plt.ylim(ymin=Y_MIN)
	if(numpy.amax(ratio) > Y_MAX):
		plt.ylim(ymax=Y_MAX)

	yl = PHYSIO_LABEL
	if(bloc==0): 	yl += " Calibrage"
	else: 			yl += " Bloc " + str(bloc)
	Ax.set(ylabel = yl, xlabel = "Temps")
	if(legend):
		Ax.legend()
	Fig.savefig(im_name, transparent = False, bbox_inches = 'tight')
	plt.close('all')

###############################################
##### Création des figures d'histogrammes #####
###############################################
def HistFig(ratio, thres, labels, bloc, im_name, legend=True):
	Fig = plt.figure(figsize = FIG_SIZE, dpi = DPI)
	Ax = plt.subplot(111)
	Ax.hist(ratio, bins = 'auto', density = True, stacked = True, label=labels[0], color=COLORS[0])
	for i in range(len(thres)):
		Ax.axvline(thres[i], label = labels[i+1], linestyle="--", color=COLORS[i+1])
	plt.xlim(xmin=Y_MIN)
	if(numpy.amax(ratio) > Y_MAX):
		plt.xlim(xmax=Y_MAX)

	xl = PHYSIO_LABEL
	if(bloc==0): 	xl += " Calibrage"
	else: 			xl += " Bloc " + str(bloc)
	Ax.set(ylabel = "Densite", xlabel = xl)
	if(legend):
		Ax.legend()
	Fig.savefig(im_name, transparent = False, bbox_inches = 'tight')
	plt.close('all')

#########################################
##### Création d'une figure de Stat #####
#########################################
def StatFig(X_axis, stats, Labels, X_Label, im_name, legend=True):
	Fig = plt.figure(figsize = FIG_SIZE, dpi = DPI)
	Ax = plt.subplot(111)
	Ax.fill_between(X_axis, stats[:,2]+stats[:,3],stats[:,2]-stats[:,3], color=COLORS[0],  alpha=0.1)
	Ax.errorbar(X_axis, stats[:,2], yerr=stats[:,3], fmt="--o", color=COLORS[0], label = Labels[0])	# Distribution
	#Ax.plot(X_axis, stats[:,0], label = Labels[1], marker="o",linestyle="--", color=COLORS[1])		# Min
	#Ax.plot(X_axis, stats[:,1], label = Labels[2], marker="o",linestyle="--", color=COLORS[4])		# Max
	Ax.yaxis.grid(True, linestyle=GRID_LINESTYLE)
	Ax.set(ylabel = "Distribution "+PHYSIO_LABEL, xlabel = X_Label)
	for axis in [Ax.xaxis]:		axis.set_major_locator(ticker.MaxNLocator(integer=True))			# Force les int sur l'axe x
	if(legend):
		Ax.legend()
	Fig.savefig(im_name, transparent = False, bbox_inches = 'tight')
	plt.close('all')

################################################
##### Création d'une figure de Recompenses #####
################################################
def RewardFig(X_axis, stats, Labels, X_Label, im_name, legend=True):
	Fig = plt.figure(figsize = FIG_SIZE, dpi = DPI)
	Ax = plt.subplot(111)
	#for j in range(4,8):	# Toutes Récompenses
	for j in range(5,7):	# Seulement Critique et Récompense
		Ax.plot(X_axis, stats[:,j], color=COLORS[j-3], marker="o",linestyle="--", label = Labels[j-1])
	Ax.set(ylabel = "Nombre d\'occurences", xlabel = X_Label)
	for axis in [Ax.xaxis, Ax.yaxis]:	axis.set_major_locator(ticker.MaxNLocator(integer=True))			# Force les int sur les axes x et y
	Ax.yaxis.grid(True, linestyle=GRID_LINESTYLE)
	if(legend):
		Ax.legend()
	Fig.savefig(im_name, transparent = False, bbox_inches = 'tight')
	plt.close('all')

##########################################
##### Création d'une figure de Score #####
##########################################
def ScoreFig(X_axis, score, X_Label, im_name, legend=True):
	Fig = plt.figure(figsize = FIG_SIZE, dpi = DPI)
	Ax = plt.subplot(111)
	Ax.plot(X_axis, score[:,-1], color=COLORS[-1], marker="o",linestyle="--", label = "Score")
	Ax.set(ylabel = "Score", xlabel = X_Label)
	for axis in [Ax.xaxis, Ax.yaxis]:	axis.set_major_locator(ticker.MaxNLocator(integer=True))			# Force les int sur les axes x et y
	Ax.yaxis.grid(True, linestyle=GRID_LINESTYLE)
	if(legend):
		Ax.legend()
	Fig.savefig(im_name, transparent = False, bbox_inches = 'tight')
	plt.close('all')


##########################################
##### Création d'une figure de Score #####
##########################################
def ThresFig(X_axis, thres, X_Label, im_name, critique=True, legend=True):
	Fig = plt.figure(figsize = FIG_SIZE, dpi = DPI)
	Ax = plt.subplot(111)
	if(critique):
		Ax.plot(X_axis, thres[:,0], color=COLORS[-3], marker="o",linestyle="--", label = "Critique")
	Ax.plot(X_axis, thres[:,1], color=COLORS[-2], marker="o",linestyle="--", label = "Recompense")
	Ax.set(ylabel = "Seuils", xlabel = X_Label)
	for axis in [Ax.xaxis]:	axis.set_major_locator(ticker.MaxNLocator(integer=True))			# Force les int sur l'axe x
	Ax.yaxis.grid(True, linestyle=GRID_LINESTYLE)
	if(legend):
		Ax.legend()
	Fig.savefig(im_name, transparent = False, bbox_inches = 'tight')
	plt.close('all')