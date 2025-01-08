import json

# Ruta al archivo JSON
file_path = 'kk.json'

# Cargar el JSON desde el archivo
with open(file_path, 'r') as file:
    json_data = file.read()

# Cargar el JSON en un diccionario
data = json.loads(json_data)

# Función para ordenar un diccionario por clave recursivamente
def sort_dict_by_key(d):
    return {k: sort_dict_by_key(v) if isinstance(v, dict) else v for k, v in sorted(d.items())}

# Normalizar y ordenar los campos
normalized_data = sort_dict_by_key(data)

# Convertir el diccionario normalizado de nuevo a JSON
normalized_json = json.dumps(normalized_data, indent=4, sort_keys=True)

print(normalized_json)

