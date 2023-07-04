"""Trinity Artifact Evaluation """


# Import the Portal object.
import geni.portal as portal
# Import the ProtoGENI library.
import geni.rspec.pg as pg
# Import the Emulab specific extensions.
import geni.rspec.emulab as emulab
import geni.rspec.pg as rspec

# Create a portal object,
pc = portal.Context()

# Create a Request object to start building the RSpec.
request = pc.makeRequestRSpec()

siteList = (
    "clemson", "utah", "apt", "umass", "wisc")
    
pc.defineParameter("nNode1", "Number of computation node 1",
                   portal.ParameterType.INTEGER, 15)

pc.defineParameter("site", "Select site", 
                   portal.ParameterType.STRING,
                   siteList[1], siteList)

pc.defineParameter("nodeType1", "The type of node 1",
                   portal.ParameterType.NODETYPE, "xl170")

pc.defineParameter("useDataset", "Use Dataset)",
                   portal.ParameterType.BOOLEAN, True)
                   
pc.defineParameter("ldatasetName", "The name of long-term dataset",
                   portal.ParameterType.STRING, "workload-dataset")

pc.defineParameter("datasetWritePerm", "Allow write to dataset (can only have at most one instance)",
                   portal.ParameterType.BOOLEAN, True)

pc.defineParameter("routableIP", "do you need a routable ip",
                   portal.ParameterType.BOOLEAN, True)

pc.defineParameter("connectNode", "Connect all nodes",
                   portal.ParameterType.BOOLEAN, False)
                   
params = pc.bindParameters()


nodes = []
lan = request.LAN("lan")

for i in range(params.nNode1):
    
    if i == 0:
        params.datasetWritePerm = True
    else:
        params.datasetWritePerm = False

    node = request.RawPC('node'+str(i))
    nodes.append(node)

    node.disk_image = "urn:publicid:IDN+emulab.net+image+emulab-ops//UBUNTU18-64-STD"
    bs_local = node.Blockstore("bs"+str(i), "/mntData")
    bs_local.size = "400GB"

    intf = node.addInterface()

    if params.routableIP:
        node.routable_control_ip = True
    else:
        node.routable_control_ip = False

    if i < params.nNode1:
        if params.nodeType1 != "None":
            node.hardware_type = params.nodeType1
    else: 
        if params.nodeType2 != "None":
            node.hardware_type = params.nodeType2

    if params.useDataset and params.ldatasetName != "None" and i == 0:
        fsnode1 = request.RemoteBlockstore("fsnode"+str(i), "/mntData2")
        fsnode1.dataset = "urn:publicid:IDN+" + params.site + ".cloudlab.us:trinity-pg0+ltdataset+"+params.ldatasetName
        # if not params.datasetWritePerm:
            # fsnode1.rwclone = True
        fsnode1.readonly = True

        fslink1 = request.Link("fslink" + str(i))
        fslink1.addInterface(intf)
        fslink1.addInterface(fsnode1.interface)

        # Special attributes for this link that we must use.
        # if params.nodeType1 == "xl170":
        #     lan.bandwidth = 25000000
        # else:
        fslink1.best_effort = True
        fslink1.vlan_tagging = True
        fslink1.link_multiplexing = True

    intf2 = node.addInterface()
    intf2.addAddress(pg.IPv4Address("10.10.1." + str(i+2), "255.255.255.0"))
    lan.addInterface(intf2)

lan.best_effort = True
lan.link_multiplexing = True
lan.vlan_tagging = True

if params.connectNode:
    for i in range(0, params.nNode1 - 1):
        for j in range(i + 1, params.nNode1 - 1):
            iface1 = nodes[i].addInterface()
            iface2 = nodes[j].addInterface()
            link   = request.Link()
            link.addInterface(iface1)
            link.addInterface(iface2)
            link.best_effort = True
            link.link_multiplexing = True

portal.context.verifyParameters()


# Print the generated rspec
pc.printRequestRSpec(request)