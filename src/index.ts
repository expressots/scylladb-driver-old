import { Cluster } from "@nodepp/example";

const cluster = new Cluster({
  nodes: ["172.21.0.2"],
});

const session = cluster.connect("system_schema");
const keyspaceName = session.executeSync<{ table_name: string }>(
  "SELECT table_name FROM scylla_tables"
);
console.log(keyspaceName);
