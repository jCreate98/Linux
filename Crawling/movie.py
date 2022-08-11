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

sql = """insert ignore into movie(m_id, m_title, v_rate, c_rate, n_rate, r_time, o_date, m_grade, m_person, m_context, m_photo) values(%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)"""
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
        title = iterate.find('h3', 'h_movie')
        v_score = iterate.find('a', 'ntz_score')
        c_score = iterate.find('a', 'spc')
        n_score = iterate.find('a', id='pointNetizenPersentBasic')

        info = iterate.find('dl', 'info_spec')
    
        print("MID: ", code.split('=')[1])
        print("제목: ", title.text.replace("상영중", "").strip())

        if v_score != None:
            if v_score.find('div', 'star_score').text.strip().split('점')[1].strip() != '없음' :
                print('관람객 평점: ' + v_score.find('div', 'star_score').text.strip().split('점')[1])
            else :
                print('관람객 평점: None')
        else :
            print('관람객 평점: Null')
        if c_score != None:
            print('평론가 평점: ', c_score.find('div', 'star_score').text.strip())
        else :
            print('평론가 평점: Null')
        if n_score != None:
            print('네티즌 평점: ', n_score.text.strip())
        else : 
            print('네티즌 평점: Null')

        if info != None:
            infos = info.find_all('p')

            details = infos[0].find_all('span')
            #scope = details[0].text.split(',')

            #for a in scope :
            #    print('장르 : ' + a.strip())
            
            #country = details[1].text.split(',')
            #for b in country :
            #    print('국가 : ' + b.strip())
            
            print('러닝 타임 : ' + details[2].text.strip())

            if len(details) > 3 :
                print('개봉 : ' + details[3].text.strip().split('재개봉,')[len(details[3].text.strip().split('재개봉,'))-1].strip().replace('\n', ''))
            

            #director = infos[1].text.split(',')
            #for b in director : 
            #    print('감독 : ' + b.strip())
            #actors = infos[2].text.split(',')
            #for c in actors :
            #    print('배우(역할)' + c.strip())

            dt_list = info.find_all('dt')
            dd_list = info.find_all('dd')

            for a in range(len(dt_list)):
                if dt_list[a].attrs['class'][0] == 'step4' :
                    if dd_list[a].text.find('국내') != -1:
                        print('등급 :' + dd_list[a].text.split("[해외]")[0].replace("[국내]", '').strip())
                        input_age = dd_list[a].text.split("[해외]")[0].replace("[국내]", '').strip()
                    else :
                        print('등급 :' + dd_list[a].text.replace("[해외]", "").replace("도움말", "").strip())
                        input_age = dd_list[a].text.replace("[해외]", "").replace("도움말", "").strip()
            

            if iterate.find('div', 'poster').find('img') != None :
                image = iterate.find('div', 'poster').find('img')['src']
                print(image)
            else :
                print("image is null")
            
            context = soup.find('p', 'con_tx')
            if context != None:
                print('줄거리 : ' + context.text)
                context = context.text
            else : 
                context = None
            

            person = info.find('div', 'step9_cont')
            if person != None:
                print('누적 관객 : ' + person.find('p', 'count').text.split('명')[0].replace(',','').strip())
                input_person = person.find('p', 'count').text.split('명')[0].replace(',','').strip()
            else :
                input_person = None

        
            input_mid = code.split('=')[1] # MID
            input_title = title.text.replace("상영중", "").strip()
            if v_score != None :
                if v_score.find('div', 'star_score').text.strip().split('점')[1].strip() != '없음' :
                    input_v_score = v_score.find('div', 'star_score').text.strip().split('점')[1]
                else :
                    input_v_score = None
            else : 
                input_v_score = None
            if c_score != None :
                input_c_score = c_score.find('div', 'star_score').text.strip()
            else : 
                input_c_score = None
            if n_score != None :
                input_n_score = n_score.text.strip()
            else : 
                input_n_score = None
            input_r_time = details[2].text.replace('분', '').strip()
            if len(details) > 3 :
                input_date = details[3].text.strip().split('재개봉,')[len(details[3].text.strip().split('재개봉,'))-1].strip().replace('\n', '').replace('개봉', '').strip()
            else :
                input_date = None
            #input_age
            #input_person

            t = (input_mid, input_title, input_v_score, input_c_score, input_n_score, input_r_time, input_date, input_age, input_person, context, image)
            buffer.append(t)

            if len(buffer) == 1:
                cur.executemany(sql, buffer)
                conn.commit()
                buffer = []

if len(buffer) != 0:
    cur.executemany(sql, buffer)
    conn.commit()

close_db(conn,cur)