import requests
import time
from bs4 import BeautifulSoup
import pymysql

start = time.time()
BASE_URL = "https://movie.naver.com/movie/bi/mi/point.naver?"

def open_db():
    conn = pymysql.connect(host='localhost', user='final_project', password = 'final_project', db='final_project')
    cur = conn.cursor(pymysql.cursors.DictCursor)
    return conn, cur

def close_db(conn, cur):
    cur.close()
    conn.close()

conn, cur = open_db()

sql = """insert ignore into mp_table(m_id, p_id, p_rtype, p_role) values(%s, %s, %s, %s)"""
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
    response = requests.get("https://movie.naver.com/movie/bi/mi/detail.naver?" + code)
    soup = BeautifulSoup(response.content, 'html.parser')
    
    actor_list = soup.find('ul', 'lst_people')
    dir_list = soup.findAll('div', 'dir_product')
    print("MID: ", code.split('=')[1])
    
    if actor_list != None:
        actor_list = actor_list.findAll('div', 'p_info')
        for li in actor_list:
            if len(li.select('a')) != 0:
                subLink = li.select('a')[0]['href']
                p_id = subLink.split('=')[1]
                print(p_id)
                role = li.find('em', 'p_part').text
                print(role)
                if li.find('p', 'pe_cmt') is not None:
                    act = li.find('p', 'pe_cmt').find('span').text        
                    print(act)
                if li.find('p', 'pe_cmt') == None:
                    act = None
            t = (code.split('=')[1], p_id, role, act)
            buffer.append(t)   

    if dir_list != None:
       dir_list = soup.findAll('div', 'dir_product')
       for div in dir_list:
            subLink = div.select('a')[0]['href']
            p_id = subLink.split('=')[1]    
            role = '감독'
            act = None   
            t = (code.split('=')[1], p_id, role, act)
            buffer.append(t)

    if len(buffer) >= 10:
        cur.executemany(sql, buffer)
        conn.commit()
        buffer = []

if len(buffer) != 0:
    cur.executemany(sql, buffer)
    conn.commit()
    
close_db(conn,cur)