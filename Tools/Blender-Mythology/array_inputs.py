import typing


def create_dynamic_input(socket_name, previous_inputs) -> typing.Any:
    links = previous_inputs[socket_name].links if socket_name in previous_inputs else []
    return {"name": socket_name, "links": links}


def create_dynamic_inputs(previous_inputs) -> typing.List[typing.Any]:

    new_inputs = []

    for input in previous_inputs:
        if len(input.links) > 0:
            new_inputs.append(
                {
                    "name": str(len(new_inputs)),
                    "from_sockets": [link.from_socket for link in input.links],
                }
            )

    new_inputs.append({"name": str(len(new_inputs)), "from_sockets": []})

    return new_inputs


def dynamic_inputs_need_to_be_recreated(inputs) -> bool:

    if len(inputs) == 0:
        return True

    if len(inputs[len(inputs) - 1].links) > 0:
        return True

    for index in range(0, len(inputs) - 1):
        input = inputs[index]
        if len(input.links) == 0:
            return True

    return False


def recreate_dynamic_inputs(node_tree, inputs, type_name) -> None:
    new_inputs = create_dynamic_inputs(inputs)
    inputs.clear()

    for input in new_inputs:
        name = input["name"]
        inputs.new(type_name, name)

        from_sockets = input["from_sockets"]
        for from_socket in from_sockets:
            node_tree.links.new(from_socket, inputs[name])


def update_dynamic_inputs(node_tree, inputs, type_name) -> None:
    if dynamic_inputs_need_to_be_recreated(inputs):
        recreate_dynamic_inputs(node_tree, inputs, type_name)
