from flask import Flask, render_template
from flask_sqlalchemy import SQLAlchemy
db = SQLAlchemy()

LSTM_PATH = 'app/LSTM/LSTMmodel.h5'

def create_app():
    app = Flask(__name__)
    app.config['SQLALCHEMY_DATABASE_URI'] = SQLALCHEMY_DATABASE_URI = 'sqlite:///MainDatabase'
    db.init_app(app)
    # Register blueprint
    from app.routes import api
    try:
        app.register_blueprint(api, url_prefix='/api')
    except Exception as e:
        return print(f"Exeption {e} occured when registering blueprint")
    #Load LSTM model
    from app.models import lst_model
    lst_model.load_model(LSTM_PATH)
    with app.app_context():
        try:
            db.create_all()
        except Exception as e:
            print(f"Got following exeprion when db.create_app() in __init__.py {e}")

    return app

