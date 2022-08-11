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

sql = """insert ignore into m_reply(r_score, r_user, r_date, r_context, m_id) values(%s, %s, %s, %s, %s)"""
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
    response = requests.get(f'https://movie.naver.com/movie/bi/mi/pointWriteFormList.naver?{code}&type=after&isActualPointWriteExecute=false&isMileageSubscriptionAlready=false&isMileageSubscriptionReject=false')
    soup = BeautifulSoup(response.content, 'html.parser')
    iterate = soup.find('div', 'score_result')

    if iterate != None:
        info = iterate.find_all('li')
    
        print("MID: ", code.split('=')[1])

        if info != None:
            i = 0
            for reply in info :
                replys = reply.find('div', 'score_reple').find_all('em')

                print('score ' + reply.find('div', 'star_score').text.strip())
                print('context ' + reply.find('div', 'score_reple').find('p').text.replace('관람객', '').strip())
                print('use ' + replys[0].text.strip())
                print('date ' + replys[1].text.strip())
                print('mid ' + code.split('=')[1])
                
                r_score = reply.find('div', 'star_score').text.strip()
                r_context = reply.find('div', 'score_reple').find('p').text.replace('관람객', '').strip()
                r_user = replys[0].text.strip()
                r_date = replys[1].text.strip()
                r_code = code.split('=')[1]

                i = i+1
                if i == 5 :
                    break
                t = (r_score, r_user, r_date, r_context, r_code)
                buffer.append(t)

            if len(buffer) >= 10:
                cur.executemany(sql, buffer)
                conn.commit()
                buffer = []

if len(buffer) != 0:
    cur.executemany(sql, buffer)
    conn.commit()

close_db(conn,cur)
