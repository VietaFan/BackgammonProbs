import logging
import os
import webapp2
import cloudstorage as gcs
import copy
from google.appengine.api import app_identity
MAX_ROLLS = 32
def getDataLine(dataFile, position, out=None):
    dataFile.seek(position*64+4)
    buf = [ord(x) for x in dataFile.read(64)]
    dist = []
    k = 0
    a = buf[1] >> 4
    b = buf[1] & 0xf
    c = buf[2] >> 4
    d = buf[2] & 0xf
    e = buf[3] >> 4
    for i in range(buf[0]):
	dist.append(0)
    x = 4
    for i in range(a):
        dist.append(0.000001*(0xff&buf[x]))
        x += 1
    for i in range(b):
        dist.append(0.000256*(0xff&buf[x])+0.000001*(0xff&buf[x+1]))
        x += 2        
    for i in range(c):
        dist.append(0.065536*(0xff&buf[x])+0.000256*(0xff&buf[x+1])+0.000001*(0xff&buf[x+2]))
        x += 3
    for i in range(d):
        dist.append(0.000256*(0xff&buf[x])+0.000001*(0xff&buf[x+1]))
        x += 2
    for i in range(e):
        dist.append(0.000001*(0xff&buf[x]))
        x += 1
    while len(dist) < MAX_ROLLS:
        dist.append(0)
    return dist
choose_dp = {}
def choose(n, k):
    if (n,k) in choose_dp:
        return choose_dp[(n,k)]
    if k == 0 or n == k:
        choose_dp[(n, k)] = 1
    elif n < k:
        choose_dp[(n, k)] = 0
    else:
        choose_dp[(n, k)] = choose(n-1, k)+choose(n-1, k-1)
    return choose_dp[(n, k)]
def convert(a):
    m = 12
    n = 15
    ans = 0
    while m > 0:
	ans += choose(n+m, m) - choose(n+m-a[m], m)# this equals (choose_dp[n+m][m]-choose_dp[n+m-a[m]][m]);
	n -= a[m]
	m = m-1
    return ans
def getPosition(request, numPoints=7):
    counts = [request.get('point%s' % (i)) for i in range(1, numPoints+1)]
    pos = [0]
    for i in range(len(counts)):
        if counts[i] == '':
            pos.append(0)
        else:
            pos.append(int(counts[i]))
    pos[0] = 15-sum(pos)
    x = 13-len(pos)
    for i in range(x):
        pos.append(0)
    return pos
def getPosFile():
    bucket_name = os.environ.get('BUCKET_NAME',
                               app_identity.get_default_gcs_bucket_name())
    #gcs_file = gcs.open('/%s/bkgmdata.txt' % (bucket_name))   
    gcs_file = open('bkgmdata.txt', 'rb')
    return gcs_file
def getMoves(position, dice, req = None, seq=[]):
    conv = convert(position)
    if not len(dice):
    #    req.write('found '+str(seq)+'<br>')
        return [conv], [seq[:]]
    inner = conv < 54264 # C(21, 6)
    d = dice[0]
    if inner:
        if position[d] > 0:
            position[d] -= 1
            position[0] += 1
            seq.append((d, 0))
            moves, seqs = getMoves(position, dice[1:], req, seq)
            seq.pop()
            position[0] -= 1
            position[d] += 1
            return moves, seqs
        if conv < choose(d+14, 15):
            k = d-1
            while not position[k]:
                k -= 1
            position[k] -= 1
            position[0] += 1
            seq.append((k, 0))
            moves, seqs = getMoves(position, dice[1:], req, seq)
            seq.pop()
            position[0] -= 1
            position[k] += 1
            return moves, seqs
    moves = []
    seqs = []
    extras = dice[1:]
    for p in range(d+1, 13):
        if not position[p]:
            continue
        position[p] -= 1
        position[p-d] += 1
        seq.append((p, p-d))
        new_moves, new_seqs = getMoves(position, extras, req, seq)
        for i in range(len(new_moves)):
            moves.append(copy.deepcopy(new_moves[i]))
            seqs.append(copy.deepcopy(new_seqs[i]))
        seq.pop()
        position[p-d] -= 1
        position[p] += 1
    return moves, seqs
        
class TopMoves(webapp2.RequestHandler):
    def begin_write(self, c=[0,0,0,0,0,0,0,0,0,0,0,0], roll=['',''], numPoints=7):
        self.response.write('<h3>Backgammon Probability Calculator</h3>')
        self.response.write('Enter the number of checkers on each point below:<br>')
        self.response.write('<form method="post" action="/topmoves">')
        for i in range(numPoints):
            self.response.write('%s <input name="point%s" type="text" value="%s"/><br>' % (i+1, i+1, c[i+1]))        
        self.response.write('Enter your roll below as a comma-separated pair.<br>')
        self.response.write('Roll <input name="roll" type="text" value="%s,%s"/><br>' % (roll[0], roll[1]))
        self.response.write('<input type="submit" value="Submit"></form>')
    def get(self):
        self.begin_write()
    def post(self):
        error = False
        try:
            c = [0]*13
            die1 = die2 = ''
            c = getPosition(self.request)
            if c[0] < 0:
                c[0] /= 0
            die1, die2 = (int(x.strip()) for x in self.request.get('roll').split(','))
            if die1 < 1 or die1 > 6 or die2 < 1 or die2 > 6:
                c[0] /= 0
        except:
            error = True
        self.begin_write(c,[die1, die2])
        if error:
            self.response.write('Incorrect input was entered.<br>')
            return
        if die1 < die2:
            die1, die2 = die2, die1
        if die1 == die2:
            moves, seqs = getMoves(c, [die1]*4, self.response)
        else:
            moves1, seqs1 = getMoves(c, [die1, die2], self.response)
            moves2, seqs2 = getMoves(c, [die2, die1], self.response)
            moves = moves1 + moves2
            seqs = seqs1 + seqs2
        moves_other = []
        seqs_other = []
        S = set()
        for i in range(len(moves)):
            if moves[i] in S:
                continue
            moves_other.append(moves[i])
            seqs_other.append(seqs[i])
            S.add(moves[i])
        moves, seqs = moves_other, seqs_other
        db = getPosFile()
        results = []
    #    self.response.write(str(moves))
        for i in range(len(moves)):
            pos = moves[i]
            try:
                probs = getDataLine(db, pos, self.response)
            except:
                probs = [0]*MAX_ROLLS
                self.response.write('error occured on '+str(pos)+'<br>')
            expVal = 0
            for n in range(MAX_ROLLS):
                expVal += n*probs[n]
            results.append((expVal, pos, seqs[i]))
        results.sort()
        self.response.write('Here are the top ways to play a %s-%s that minimize the expected number of rolls.<br>' % (die1, die2))
        self.response.write('<style> td, th {border: 1px solid #000000;}</style>')
        self.response.write('<table><tr><td>Play</td><td>Expected Value</td></tr>')
        for i in range(min(10, len(results))):
            self.response.write('<tr><td>')
            self.response.write(', '.join(['%s-%s' % (a,b) for a,b in sorted(results[i][2], reverse=True)]))
            self.response.write('</td><td>')
            s = str(round(results[i][0], 5))
            self.response.write(s+'0'*(6-(len(s)-s.find('.'))))
            self.response.write('</td></tr>')
        self.response.write('</table>')
class Root(webapp2.RequestHandler):
    def begin_write(self, c=[0,0,0,0,0,0,0,0,0,0,0,0], numPoints=7):
        self.response.write('<h3>Backgammon Probability Calculator</h3>')
        self.response.write('Enter the number of checkers on each point below:\n')
        self.response.write('<form method="post" action="/">')
        for i in range(numPoints):
            self.response.write('%s <input name="point%s" type="text" value="%s"/><br>' % (i+1, i+1, c[i+1]))
        self.response.write('<input type="submit" value="Submit"></form>')
    def get(self):
        self.begin_write()
    def post(self):
        c = getPosition(self.request)
        self.begin_write(c)
        k = convert(c)
        bucket_name = os.environ.get('BUCKET_NAME',
                               app_identity.get_default_gcs_bucket_name())
        #gcs_file = gcs.open('/%s/bkgmdata.txt' % (bucket_name))   
        gcs_file = open('bkgmdata.txt')
        line = getDataLine(gcs_file, k)
        pieces = []
        for i in range(len(line)):
            if line[i] > 0:
                pieces.append([str(i), line[i]])
        dataArray = [["Element", "Probability"]] + pieces
        self.response.write('''<html>
  <head>
    <!--Load the AJAX API-->
    <script type="text/javascript" src="https://www.gstatic.com/charts/loader.js"></script>
    <script type="text/javascript">
     var data;
     var chart;

      // Load the Visualization API and the piechart package.
      google.charts.load('current', {'packages':['corechart']});

      // Set a callback to run when the Google Visualization API is loaded.
      google.charts.setOnLoadCallback(drawChart);

      // Callback that creates and populates a data table,
      // instantiates the pie chart, passes in the data and
      // draws it.
      function drawChart() {
        var data = google.visualization.arrayToDataTable(%s);
        var options = {'title':'Probabilities of Rolls',
                       'vAxis': {format: 'percent'},
                       'hAxis': {title: 'Number of Rolls'},
                       'width':700,
                       'height':500};

        // Instantiate and draw our chart, passing in some options.
        chart = new google.visualization.ColumnChart(document.getElementById('chart_div'));
        google.visualization.events.addListener(chart, 'select', selectHandler);
        chart.draw(data, options);
      }

      function selectHandler() {
        var selectedItem = chart.getSelection()[0];
        var value = data.getValue(selectedItem.row, 0);
        alert('The user selected ' + value);
      }

    </script>
  </head>
  <body>
    <!--Div that will hold the pie chart-->
    <div id="chart_div" style="width:700; height:500"></div>
  </body>
</html>
''' % (str(dataArray)))

# [START app]
app = webapp2.WSGIApplication([
    ('/', Root),
    ('/topmoves', TopMoves)
], debug=True)
# [END app]
