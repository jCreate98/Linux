import requests
import time
from bs4 import BeautifulSoup
import pymysql

start = time.time()
BASE_URL = "https://movie.naver.com/movie/bi/mi/basic.naver?"

def open_db():
    conn = pymysql.connect(host='localhost', user='final_project', password = 'final_project', db='final_project')
    cur = conn.cursor(pymysql.cursors.DictCursor)
    return conn, cur

def close_db(conn, cur):
    cur.close()
    conn.close()

conn, cur = open_db()

sql = """insert ignore into mg_table(g_name, m_id) values(%s, %s)"""
buffer = []

movie_list = []
for j in range(1, 41):
    response = requests.get("https://movie.naver.com/movie/sdb/rank/rmovie.naver?sel=pnt&tg=0&date=20220611&tg=1&page=" + str(j))
    soup = BeautifulSoup(response.content, 'html.parser')
    #220611 220101 210701 210101 200701 200101 
    lists = soup.find('table', 'list_ranking').find('tbody').find_all('td', 'title')

    for k in range(len(lists)):
        movie_list.append(lists[k].find('a')['href'].split('?')[1])

for code in movie_list:
    response = requests.get(BASE_URL + code)
    soup = BeautifulSoup(response.content, 'html.parser')
    
    iterate = soup.find('div', 'mv_info_area')

    if iterate != None:
        info = iterate.find('dl', 'info_spec')
    
        print("MID: ", code.split('=')[1])

        if info != None:
            infos = info.find_all('p')

            details = infos[0].find_all('span')
            scope = details[0].text.split(',')

            for a in scope :
                print('장르 : ' + a.strip())
                t = (a.strip(), code.split('=')[1])
                buffer.append(t)             
            
            #country = details[1].text.split(',')
            #for b in country :
            #    print('국가 : ' + b.strip())

            if len(buffer) >= 10:
                cur.executemany(sql, buffer)
                conn.commit()
                buffer = []

if len(buffer) != 0:
    cur.executemany(sql, buffer)
    conn.commit()

close_db(conn,cur)