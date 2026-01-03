// 游戏逻辑示例
function calculateDamage(base, multiplier, critical) {
    let damage = base * multiplier;
    if (critical) {
        damage = damage * 2;
    }
    return damage;
}

// 玩家对象
let player = {
    name: "Ash",
    level: 25,
    health: 100
};

// 计算战斗伤害
let attackPower = 50;
let defense = 20;
let isCritical = true;

let finalDamage = calculateDamage(attackPower, 1.5, isCritical);
let remainingHealth = player.health - finalDamage;

// 道具数组
let items = ["Potion", "Antidote", "Paralyze Heal"];

// 循环处理道具
for (let i = 0; i < items.length; i++) {
    let itemName = items[i];
}

// 条件判断
if (remainingHealth <= 0) {
    player.health = 0;
} else {
    player.health = remainingHealth;
}
