from __future__ import division
from sklearn.model_selection import train_test_split
from sklearn import tree
from sklearn.metrics import confusion_matrix
from IPython.display import Image
import pydotplus
import matplotlib.pyplot as plt
import itertools
import pandas as pd
import numpy as np

def processData(raw_data):
	num_cols 		= len(raw_data.columns)
	features 		= raw_data.loc[:,1:num_cols-2]
	labels	 		= raw_data.loc[:,num_cols-1]
	features_list 	= features.values.tolist()
	labels_list		= labels.values.tolist()
	return [features_list,labels_list]

def prepareData(df):
	df.columns = range(len(df.columns))
	#calculate the returns between pred_exe and curr_exe
	temp_ds = df[5].diff()/df[5][1:]
	temp_ds = temp_ds * 100000
	v = np.array([-2.3, 0.0, 2.3])
	count = 0
	for elem in temp_ds:
		temp_ds[count] = np.argmin(np.abs(elem-v))
		count = count + 1
	df = df.drop(df.columns[[0,5]],axis=1)
	df[len(df.columns)] = temp_ds
	return df[1:] 


##========================================##
##This function is from python online sample##
def plot_confusion_matrix(cm, classes,
                          normalize=False,
                          title='Confusion matrix',
                          cmap=plt.cm.Blues):
    """
    This function prints and plots the confusion matrix.
    Normalization can be applied by setting `normalize=True`.
    """
    plt.imshow(cm, interpolation='nearest', cmap=cmap)
    plt.title(title)
    plt.colorbar()
    tick_marks = np.arange(len(classes))
    plt.xticks(tick_marks, classes, rotation=45)
    plt.yticks(tick_marks, classes)

    if normalize:
        cm = cm.astype('float') / cm.sum(axis=1)[:, np.newaxis]
        print("Normalized confusion matrix")
    else:
        print('Confusion matrix, without normalization')

    print(cm)

    thresh = cm.max() / 2.
    for i, j in itertools.product(range(cm.shape[0]), range(cm.shape[1])):
        plt.text(j, i, cm[i, j],
                 horizontalalignment="center",
                 color="white" if cm[i, j] > thresh else "black")

    plt.tight_layout()
    plt.ylabel('True label')
    plt.xlabel('Predicted label')

##========================================##


path = 'traning_data.csv'
df = prepareData(pd.read_csv(path))

df.columns = range(len(df.columns))
split_index = int(len(df)*0.7)
train_df = df.loc[0:split_index, :]
test_df = df.loc[split_index+1:len(df),:]
train_df, _test_df = train_test_split(train_df, test_size = 0.0)
train_data = processData(train_df)
test_data = processData(test_df)
clf = tree.DecisionTreeClassifier(max_depth=3)
clf = clf.fit(train_data[0], train_data[1])
print clf.score(test_data[0], test_data[1])
y_pred = clf.fit(train_data[0],train_data[1]).predict(test_data[0])
np.savetxt("y_pred.csv", y_pred, delimiter=",")
cnf_matrix = confusion_matrix(test_data[1], y_pred)
class_names = ["down","---","up"]
# Plot non-normalized confusion matrix
plt.figure()
plot_confusion_matrix(cnf_matrix, classes=class_names,
                      title='Confusion matrix, without normalization')
plt.show()
