
import subprocess, os

env = Environment()
env.Append(CPPPATH=['libs/sfml/include', 'libs/jsoncpp/include'])
env.Append(LIBPATH=['libs/sfml/lib',])
env.Append(LIBS=['sfml-graphics', 'sfml-window', 'sfml-system', 'jsoncpp', 'PocoFoundation', 'PocoNet'])
env.Append(CXXFLAGS=['-std=c++11'])

# Directories relative to src
dirs = ['/', '/graphics/', '/logic/', '/net/', '/state/']
sources = []

for dir in dirs:
	sources += Glob('src' + dir + '*.cpp')

Mkdir('build')
program = env.Program('build/hack', sources)
env.Default(program)

#os.environ['LD_LIBRARY_PATH'] = os.environ["LD_LIBRARY_PATH"] + 'libs/sfml/lib'
#, env=os.environ['LD_	LIBRARY_PATH'])

#def runHack():
#	subprocess.Popen('LD_LIBRARY_PATH=libs/sfml/lib:$LD_LIBRARY_PATH build/hack')
	
#Alias('run', Command(runHack))



