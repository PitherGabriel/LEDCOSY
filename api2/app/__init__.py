from flask import Flask, render_template
from flask_sqlalchemy import SQLAlchemy
import pandas as pd
db = SQLAlchemy()

LSTM_PATH = 'app/static/LSTMmodel.h5'
TESTDATA_PATH = 'app/static/TestDataset.csv'

def create_app():
    app = Flask(__name__)
    app.config['SQLALCHEMY_DATABASE_URI'] = SQLALCHEMY_DATABASE_URI = 'sqlite:///MainDatabase'
    app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False

    db.init_app(app)
    # Register blueprint
    from app.routes import api
    try:
        app.register_blueprint(api, url_prefix='/api')
    except Exception as e:
        return print(f"Exeption {e} occured when registering blueprint")
    #Load LSTM model
    from app.models import lst_model, test_table
    lst_model.load_model(LSTM_PATH)

    #with open (TESTDATA_PATH, newline='') as csvfile:
    #    df = pd.read_csv(csvfile)
  
    with app.app_context():
        try:
            db.create_all()
            #df.to_sql(test_table.__tablename__, db.engine, if_exists='replace')
        except Exception as e:
            print(f"Got following exeprion when db.create_app() in __init__.py {e}")

    return app

