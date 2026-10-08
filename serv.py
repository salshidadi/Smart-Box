from flask import Flask, request, render_template
from flask_socketio import SocketIO    


app = Flask(__name__)
socketio = SocketIO(app, cors_allowed_origins="*")
#web path
@app.route('/')
def index():
    return render_template('index.html')

#esp path
@app.route('/api/sensor', methods=['POST'])
def receive_sensor_data():
    data = request.get_json()
    
    if not data:
        return "Invalid JSON", 400
        
    print(f"reseved: {data}")
    
    socketio.emit('sensor_update', data)
    
    return "OK", 200

if __name__ == '__main__':
    socketio.run(app, host='0.0.0.0', debug=True, port=5000)



