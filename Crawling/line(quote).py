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

sql = """insert ignore into m_line(l_img, l_line, l_name, p_id, m_id) values(%s, %s, %s, %s, %s)"""
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
    response = requests.get(f'https://movie.naver.com/movie/bi/mi/script.naver?{code}&type=after&isActualPointWriteExecute=false&isMileageSubscriptionAlready=false&isMileageSubscriptionReject=false')
    soup = BeautifulSoup(response.content, 'html.parser')
    iterate = soup.find('div', 'ifr_area')

    if iterate != None:
        info = iterate.find('ul', 'lines')
        if info != None:
            info = info.find_all('li')
    
        print("MID: ", code.split('=')[1])
        if info != None:
            i = 0
            for reply in info :
                imgs = reply.find('p', 'thumb').find('a').find('img')['src']
                lines = reply.find('p', 'one_line').text
                who = reply.find('div').find('a').text.strip()
                p_id = reply.find('div').find('a')['href'].split('=')[1]

                i = i+1

                print(imgs)
                print(lines)
                print(who)
                print(p_id)

                t = (imgs, lines, who, p_id, code.split('=')[1])
                buffer.append(t)

                if i == 5 :
                    break
                
            if len(buffer) >= 10:
                cur.executemany(sql, buffer)
                conn.commit()
                buffer = []

if len(buffer) != 0:
    cur.executemany(sql, buffer)
    conn.commit()
close_db(conn,cur)