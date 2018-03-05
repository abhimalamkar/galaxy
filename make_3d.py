'''
 Copyright (C) 2018 Greenweaves Software Pty Ltd

 This is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This software is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this software.  If not, see <http://www.gnu.org/licenses/>

  Companion to test-kepler.cpp - plot images
'''
import os, re, sys, numpy as np, matplotlib.pyplot as plt,mpl_toolkits.mplot3d as trid,getopt, matplotlib.lines as lines, scipy, random

colours=['r','b','g','c','y','m']

def get_limits(xs,nsigma=3):
    mu=scipy.mean(xs)
    sigma=scipy.std(xs)
    return (mu-nsigma*sigma,mu+nsigma*sigma)

'''
Plot points from input file
    Parameters:
	    fname_in  Input file
		n         Number of bodies
        
I have had to workaround the problem of the legend function not supporting the type returned by a 3D scatter.
See https://stackoverflow.com/questions/20505105/add-a-legend-in-a-3d-scatterplot-with-scatter-in-matplotlib for details.
'''
def plot(fname_in='kepler.csv',n=len(colours),m=sys.maxsize,scale_to_cube=False,out='./imgs',nsigma=3,N=1000):    
    pos = np.loadtxt(fname_in,delimiter=',')
    plt.figure(figsize=(20,10))
    ax = plt.gcf().add_subplot(111, aspect='equal', projection='3d')
    scatterproxies=[]
    labels=[]
    for i in range(n):
        full_range=list(range(i,min(len(pos),m),n))
        sample=full_range if N==None else random.sample(full_range,N)
        xs=[pos[j,0] for j in sample]
        ys=[pos[j,1] for j in sample]
        zs=[pos[j,2] for j in sample]
        colour=colours[i%len(colours)]
        ax.scatter(xs,ys,zs,edgecolor=colour,s=1)
        if scale_to_cube:
            ax.set_xlim(get_limits(xs,nsigma=nsigma))
            ax.set_ylim(get_limits(ys,nsigma=nsigma))
            ax.set_zbound(get_limits(zs,nsigma=nsigma))
        scatterproxies.append( lines.Line2D([0],[0], linestyle="none", c=colour, marker = 'o'))
        labels.append('{0}'.format(i))
    plt.title(fname_in.replace('.csv',''));
    plt.xlabel('x')
    plt.ylabel('y')
    
    ax.legend(scatterproxies,labels, numpoints = 1) 
    img_file=os.path.join(out,os.path.basename(fname_in).replace('.csv','.png'))
    plt.savefig(img_file)
    return img_file
    
def usage():
    print ('python make_img.py -h -s [-m digits] [-n digits] file1.csv [file2.csv...]')

if __name__=='__main__':
    try:
        out           = './imgs'
        n             = len(colours)
        m             = sys.maxsize
        show          = False
        scale_to_cube = False
        i             = 0
        img_freq      = 20
        nsigma        = 3
        N             = 1000
        
        opts, args = getopt.getopt(sys.argv[1:], 'hm:n:scog:N:', ['help', 'points=','bodies=','show','cube','out=','nsigma=','sample='])
        for o,a in opts:
            if o in ['-h','--help']:
                usage()
                sys.exit(0)
            elif o in ['-n','--bodies']:
                n=int(a)
            elif o in ['-N','--sample']:
                N=None if a=='all' else int(a)            
            elif o in ['-m','--points']:
                m=int(a)            
            elif o in ['-s','--show']:
                show=True
            elif o in ['-c','--cube']:
                scale_to_cube=True
            elif o in ['-o','--out']:
                out=a
            elif o in ['-g','--nsigma']:
                nsigma=float(a)              
            else:
                print ('Unexpected option {0} {1}'.format(o,a))
                sys.exit(2) 
        for fname_in in args:
            if os.path.isfile(fname_in):
                img_file=plot(fname_in=fname_in,n=n,m=m,scale_to_cube=scale_to_cube,out=out,N=N,nsigma=nsigma)
                if i%img_freq==0:
                    print ('Created {0}'.format(img_file))
                i+=1
                if not show:
                    plt.close()                
            elif os.path.isdir(fname_in):
                for filename in os.listdir(fname_in):
                    if filename.endswith(".csv"):
                        img_file=plot(fname_in=os.path.join(fname_in,filename),n=n,m=m,scale_to_cube=scale_to_cube,out=out,N=N,nsigma=nsigma)
                        if i%img_freq==0:
                            print ('Created {0}'.format(img_file))
                        i+=1
                        if not show:
                            plt.close()
        if show:
            plt.show()
    except getopt.GetoptError as err:  # print help information and exit:      
        print(err)  # will print something like "option -a not recognized"
        usage()
        sys.exit(2)    