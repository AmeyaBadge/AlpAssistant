from flask import Flask, jsonify
import threading
import speech_recognition as sr
from googletrans import Translator

app = Flask(__name__)

# Shared variables
recognized_text = None
is_processing = False
lock = threading.Lock()

def recognize_speech_background():
    global recognized_text, is_processing

    recognizer = sr.Recognizer()
    mic = sr.Microphone()
    translator = Translator()

    try:
        with mic as source:
            print("Calibrating mic for ambient noise...")
            recognizer.adjust_for_ambient_noise(source, duration=1)
            print("Listening... Speak now!")

            audio = recognizer.listen(source, phrase_time_limit=8)
            print("Audio captured. Recognizing...")

        raw_text = recognizer.recognize_google(audio, show_all=False)
        print("Raw recognized:", raw_text)

        translated = translator.translate(raw_text, dest='en')
        final_text = translated.text.strip()
        print("Translated to English:", final_text)

        with lock:
            recognized_text = final_text
            is_processing = False

    except Exception as e:
        print("Recognition error:", str(e))
        with lock:
            recognized_text = ""
            is_processing = False

@app.route('/start-listening', methods=['GET'])
def start_listening():
    global is_processing, recognized_text

    with lock:
        if is_processing:
            return jsonify({"message": "Already listening"}), 429  # Too many requests
        recognized_text = None
        is_processing = True

    # Start recognition in a separate thread
    threading.Thread(target=recognize_speech_background).start()
    return jsonify({"message": "Listening started"}), 200

@app.route('/result', methods=['GET'])
def get_result():
    with lock:
        if recognized_text is not None:
            return jsonify({"text": recognized_text}), 200
        elif is_processing:
            return jsonify({"message": "Still processing"}), 202
        else:
            return jsonify({"message": "No result available"}), 404

@app.route('/status')
def status():
    return jsonify({"message": "Flask server running"}), 200

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
