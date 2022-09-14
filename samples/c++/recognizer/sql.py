from cProfile import run
import sys
import pymssql

query = sys.argv[1]

def runQuery(query):
	con =  pymssql.connect( server="" , 
        port="", 
        user="", 
        password="", 
        database="", 
        charset='utf8', as_dict=True)
	curr = con.cursor()
	curr.execute(query)
	con.commit()
	con.close()


runQuery(query)