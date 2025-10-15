from flask import Flask, jsonify,request
import numpy as np
import random
import time


app = Flask(__name__)


@app.route("/problem", methods=['GET'])
def problem():
  dim = request.args.get("size", None)
  dim = int(np.random.rand() * 11) * 2 + 4 if dim is None else int(dim)

  entities = list(range(int(dim*dim/2)))
  entities.extend(range(int(dim*dim/2)))
  random.shuffle(entities)
  
  entities = np.array(entities).reshape(dim, dim)
  

  ret = {
    "startsAt": int(time.time()),
    "problem": {
      "field": {
        "size": dim,
        "entities": entities.tolist()
      }
    }
  }

  return jsonify(ret), 200

if __name__ == "__main__":
  app.run()
